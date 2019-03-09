// Copyright © 2019 Brad Ackerman.
// Licensed under the MIT License (LICENSE.txt in this repository).

#pragma once

#include <QByteArray>
#include <QBuffer>
#include <QMutex>
#include <QObject>
#include <QSerialPort>
#include <QTimer>

#include <memory>

class TestLink; // forward-declare test class for friendship

namespace CommsLink {
enum struct PacketType : uint8_t {
    acknowledge = 0,
    disconnect  = 1,
    linkRequest = 2,
    data        = 3
};

struct Message {
    PacketType type;
    uint8_t    sequenceNo;
    QByteArray data;
};

class Link : public QObject
{
    Q_OBJECT
private:
    QIODevice *port = nullptr;
    QTimer *readTimer = nullptr;
    QBuffer writeBuf;
    QByteArray readBuf;
    QMutex busy;
    /// \brief The number of bytes remaining to write in the current packet.
    qint64 numBytesToWrite = 0;
    /// \brief The sequence number of the next packet to be sent. it's a uint64 here
    /// to eliminate an alignment warning.
    quint64 nextSeq = 0;
    bool validMessageReceived();
    /// \brief Read a message from the buffer.
    /// \param popCompleteMessage Iff true and a valid and complete
    /// message is in the buffer, clear the buffer afterwards.
    /// \return The message, or nullptr if none available.
    std::unique_ptr<Message> parseMessage(bool popCompleteMessage = true);
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
    void packetReceived(Message);

public slots:
    void bytesWritten(qint64 numBytes);
    /// \brief Notify this object that bytes are available for reading on the port.
    void readyRead();
    void readTimeout();

friend class ::TestLink;
};
}
