// Copyright © 2019 Brad Ackerman.
// Licensed under the MIT License (LICENSE.txt in this repository).

#include "link.hpp"
#include <array>
#include <QDebug>
#include <QTextStream>

namespace CommsLink {
Link::Link(QObject *parent) : QObject(parent)
{
}

constexpr char packetStart[]{0x16, 0x10, 0x02};
constexpr char dataEnd[]{0x10, 0x03};

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
    connect(port, &QIODevice::bytesWritten,
            this, &Link::bytesWritten);
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
    quint8 seqAndType = static_cast<quint8>(
                (static_cast<quint8>(pType) << 3)
                | static_cast<quint8>(nextSeq));
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

    // Increment sequence number for next packet.
    nextSeq++;

    return true;
}

void Link::bytesWritten(qint64 numBytes) {
    if (numBytes > numBytesToWrite) {
        // NOPE.
    }
    numBytesToWrite -= numBytes;
    if (numBytesToWrite == 0) {
        /// This packet has been completely written.
        busy.unlock();
    }
}

}
