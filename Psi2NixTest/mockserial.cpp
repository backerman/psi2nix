// Copyright © 2019 Brad Ackerman.
// Licensed under the MIT License (LICENSE.txt in this repository).

#include "mockserial.hpp"

#include <cstring>
#include <QDebug>

MockSerial::MockSerial(QObject *parent) : QIODevice(parent)
{
    sendBuf.open(QBuffer::ReadWrite);
    recvBuf.open(QBuffer::ReadWrite);
    connect(&recvBuf, &QIODevice::readyRead,
            this, &QIODevice::readyRead);
    connect(&sendBuf, &QIODevice::bytesWritten,
            this, &QIODevice::bytesWritten);
}

qint64 MockSerial::readData(char *data, qint64 maxSize) {
    qDebug() << "MockSerial::readData called with maxSize"
             << maxSize;
    if (maxSize == 0) {
        return 0;
    }
    if (maxSize < 0) {
        return -1;
    }
    qDebug() << "recvBuf pos" << recvBuf.pos()
             << "; available " << recvBuf.bytesAvailable();
    auto actualReadLength = recvBuf.read(data, maxSize);
    qDebug() << "Actual read length:" << actualReadLength;
    return actualReadLength;
//    const char *source = recvBuf.data();
//    char *dest   = data;
//    qint64 copyLength = recvBuf.size();
//    if (maxSize < copyLength) {
//        copyLength = maxSize;
//    }
//    memcpy(dest, source, static_cast<size_t>(copyLength));
//    recvBuf.remove(0, static_cast<int>(copyLength));
//    return copyLength;
}

qint64 MockSerial::writeData(const char *data, qint64 maxSize) {
    return sendBuf.write(data, maxSize);
//    sendBuf.append(data, static_cast<int>(maxSize));
//    qDebug() << "Yeah, this is where I emit.";
//    emit bytesWritten(maxSize);
//    qDebug() << "Returned from emission.";
//    return maxSize;
}

void MockSerial::sendData(const QByteArray &data) {
    sendData(data.data(), data.size());
}

void MockSerial::sendData(const char *data, qint64 size) {
    qDebug() << "sendData called with size" << size;
    auto bytesWritten = recvBuf.write(data, size);
    qDebug() << "bytes written:" << bytesWritten
             << "; pos: " << recvBuf.pos();
    recvBuf.seek(0);
    emit readyRead();
    qDebug() << "readyRead emitted.";
}

qint64 MockSerial::bytesAvailable() const {
    return recvBuf.bytesAvailable();
}

