// Copyright © 2019 Brad Ackerman.
// Licensed under the MIT License (LICENSE.txt in this repository).

#pragma once

#include <QByteArray>
#include <QBuffer>
#include <QMutex>
#include <QObject>
#include <QSerialPort>

#include <memory>

namespace CommsLink {
enum struct PacketType : uint8_t {
    acknowledge = 0,
    disconnect  = 1,
    linkRequest = 2,
    data        = 3
};

class Link : public QObject
{
    Q_OBJECT
private:
    QIODevice *port = nullptr;
    QBuffer writeBuf;
    QMutex busy;
    /// \brief The number of bytes remaining to write in the current packet.
    qint64 numBytesToWrite = 0;
    /// \brief The sequence number of the next packet. it's a uint64 here
    /// to eliminate an alignment warning.
    quint64 nextSeq = 1;

public:
    explicit Link(QObject *parent = nullptr);
    /// \brief Send the contents of data to the device.
    /// \param type The packet type to send.
    /// \param data The data to send.
    /// \return true if we could start the send; false if the link
    /// was busy.
    bool send(PacketType type, const QByteArray &data);
    void setPort(QIODevice &port);
signals:

public slots:
    void bytesWritten(qint64 numBytes);
};
}
