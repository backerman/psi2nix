// Copyright © 2019 Brad Ackerman.
// Licensed under the MIT License (LICENSE.txt in this repository).

#include "link.hpp"
#include <array>
#include <QDebug>
#include <QTextStream>

namespace CommsLink {

// Minimum and maximum sizes for a received message.
const int MIN_MSG_SIZE = 0x009;
const int MAX_MSG_SIZE = 0x200;
// Timeout value - maximum time without any data received
// before input buffer flushed.
const std::chrono::milliseconds timeoutValue{250};

// Start and end sequences
constexpr char packetStart[]{0x16, 0x10, 0x02};
constexpr char dataEnd[]{0x10, 0x03};
const QByteArray postamble(dataEnd, sizeof(dataEnd));

Link::Link(QObject *parent) : QObject(parent)
{
    // add one for the '\0' terminator.
    readBuf.reserve(MAX_MSG_SIZE + 1);

    readTimer = new QTimer(this);
    connect(readTimer, &QTimer::timeout,
            this, &Link::readTimeout);
}

// Generate the lookup table for our CRC-16 function.
constexpr quint16 CRC16_POLYNOMIAL = 0xa001;
constexpr quint16 crc16Byte(quint8 b) {
    quint16 c = b;
    quint16 crc = 0x0000;
    for (quint16 j = 0; j<8; j++) {
        if ((crc^c)& 0x0001) {
            crc = static_cast<quint16>((crc >> 1) ^ CRC16_POLYNOMIAL);
        } else {
            crc >>= 1;
        }
        c >>= 1;
    }
    return static_cast<quint16>(((crc & 0xff) << 8) | (crc>>8));
}

constexpr std::array<quint16, 256> initCRC16Table() {
    std::array<quint16, 256> res{0};
    for (quint16 i = 0; i < 256; i++) {
        res[i] = crc16Byte(static_cast<quint8>(i));
    }
    return res;
}

constexpr std::array<quint16, 256> crc16table = initCRC16Table();

void Link::setPort(QIODevice &aPort) {
    // TODO disconnect bytesWritten from old?
    port = &aPort;
    port->open(QIODevice::ReadWrite);
    connect(port, &QIODevice::bytesWritten,
            this, &Link::bytesWritten);
    connect(port, &QIODevice::readyRead,
            this, &Link::readyRead);
}

bool Link::send(PacketType pType, const QByteArray &data) {
    if (!busy.tryLock()) {
        // Unable to acquire a lock.
        return false;
    }
    // As it happens, qChecksum calculates the CRC-16 using the CCITT
    // polynomial, which is exactly what the Psion protocol uses.
    //
    // Sadly, since we want to ignore byte-stuffing and also checksum the
    // channel/type/sequence bytes, we'd need to generate a QByteArray
    // with the c/t/s and then an un-byte-stuffed copy of data if we were
    // to use qChecksum. So I'm DIYing here.
    quint16 checksum = 0x0000;
    const auto checkByte = [&](quint8 b){
        checksum = static_cast<quint16>((checksum << 8)
                                        ^ crc16table[b ^ (checksum >> 8)]);
    };
    writeBuf.open(QBuffer::ReadWrite);
    writeBuf.write(packetStart, sizeof(packetStart));
    writeBuf.putChar(0x01); // channel number
    checkByte(0x01);
    // Increment sequence number for next packet - only if data.
    quint8 thisSeq = 0;
    if (pType == PacketType::data) {
        nextSeq++;
        thisSeq = static_cast<quint8>(nextSeq);
    }
    quint8 seqAndType = static_cast<quint8>(
                (static_cast<quint8>(pType) << 3)
                | thisSeq);
    writeBuf.putChar(static_cast<char>(seqAndType));
    checkByte(seqAndType);
    if (seqAndType == 0x10) {
        // Escape it.
        writeBuf.putChar(static_cast<char>(seqAndType));
    }
    // Write data, escaping 0x10.
    for (auto b : data) {
        writeBuf.putChar(b);
        checkByte(static_cast<quint8>(b));
        // Any 0x10 byte is repeated for sending.
        if (b == 0x10) {
            writeBuf.putChar(b);
        }
    }
    writeBuf.write(dataEnd, sizeof(dataEnd));
    // Append checksum.
    writeBuf.putChar(static_cast<char>(checksum >> 8));
    writeBuf.putChar(static_cast<char>(checksum & 0xff));

    // Call port.write method. Ensure that this method can't be called again
    // until write finishes or times out.
    numBytesToWrite = writeBuf.size();
    port->write(writeBuf.data());

    return true;
}

void Link::bytesWritten(qint64 numBytes) {
    if (numBytes > numBytesToWrite) {
        // NOPE.
        qDebug() << "Overwrite: " << numBytesToWrite << "remaining;"
                 << numBytes << "allegedly written";
    }
    numBytesToWrite -= numBytes;
    qDebug() << numBytes << "written; " << numBytesToWrite << "remaining.";
    if (numBytesToWrite == 0) {
        /// This packet has been completely written.
        busy.unlock();
    }
}

void Link::readyRead() {
    qint64 bytesAvail = port->bytesAvailable();
    qDebug() << "Bytes available to read:" << bytesAvail;
    if ((readBuf.size() + bytesAvail) > MAX_MSG_SIZE) {
        // This cannot happen. Emit error.
        return;
    }

    // (Re)set the timeout period.
    readTimer->start(timeoutValue);
    readBuf.append(port->readAll());
    qint64 bytesRead = bytesAvail - port->bytesAvailable();
    if (bytesRead < bytesAvail) {
        qWarning() << "Unexpected short read";
    }
    auto msg = parseMessage();
    if (msg) {
        // There's a valid message here.
        readTimer->stop();
        emit packetReceived(*msg);
    }
}

bool Link::validMessageReceived() {
    return parseMessage(false) == nullptr;
}

std::unique_ptr<Message> Link::parseMessage(bool popCompleteMessage){
    // For now, assume that buffer contains only an actual frame.
    // Check for preamble.

    if (readBuf.size() < MIN_MSG_SIZE
            || readBuf.at(0) != 0x16
            || readBuf.at(1) != 0x10
            || readBuf.at(2) != 0x02) {
        qDebug() << "Not enough data to be a message:"
                 << readBuf.size() << "byte(s) received.";
        return nullptr;
    }
    // Check valid channel, type, sequence.
    // Check for 0x10 03 and CRC afterwards
    auto postamblePos = readBuf.indexOf(postamble);
    if (postamblePos < 0 || readBuf.size() < postamblePos + 3) {
        // postamblePos starts 2-byte postamble and 2-byte CRC;
        // therefore the last byte of the frame is at postamblePos + 3
        qDebug() << "Postamble but no CRC";
        return nullptr;
    }
    // Validate CRC
    quint16 checksum = 0x0000;
    const auto checkByte = [&](quint8 b){
        checksum = static_cast<quint16>((checksum << 8)
                                        ^ crc16table[b ^ (checksum >> 8)]);
    };
    bool escaped = false;
    for (auto i = 3; i < postamblePos; i++) {
        quint8 b = static_cast<quint8>(readBuf.at(i));
        if (escaped) {
            // this is an 0x10; skip it.
            Q_ASSERT(b == 0x10);
            escaped = false;
            continue;
        }
        checkByte(b);
        if (b == 0x10) {
            // skip the next byte (0x10)
            escaped = true;
        }
    }
    quint16 expectedChecksum =
            static_cast<quint16>((readBuf.at(postamblePos + 2 ) << 8)
                                 | (readBuf.at(postamblePos + 3)));
    if (expectedChecksum != checksum) {
        qWarning() << "Frame received with bad CRC";
        return nullptr;
    }
    // Emit signal for available data
    // ACK or NAK.
    qDebug() << "Frame received with good CRC";
    auto preamblePos = 0; // for searching the whole buffer later
    // Preamble is three bytes, then one-byte channel number.
    auto typePos = preamblePos + 4;
    auto msg = std::make_unique<Message>();
    auto typeField = readBuf.at(typePos);
    msg->type = static_cast<PacketType>(typeField >> 3);
    msg->sequenceNo = typeField & 0x7;
    if (typeField == 0x10) {
        // Start the copy loop below at the right position.
        typePos++;
    }
    // Append data, de-escaping.
    Q_ASSERT(postamblePos < readBuf.size());
    for (auto pos = readBuf.cbegin() + typePos + 1;
         pos < readBuf.cbegin() + postamblePos; pos++) {
        char curByte = *pos;
        msg->data.append(curByte);
        if (curByte == 0x10) {
            // Advance a second time.
            pos++;
        }
    }
    if (popCompleteMessage) {
        readBuf.truncate(0);
    }
    return msg;
}

void Link::readTimeout() {
    qDebug() << "Read timeout reached; flushing buffer";
    port->readAll();
    readBuf.clear();
}

}
