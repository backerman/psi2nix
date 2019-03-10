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
/// \brief The available packet types.
enum struct PacketType : uint8_t {
    acknowledge = 0,
    disconnect  = 1,
    linkRequest = 2,
    data        = 3,
    unknown     = 255
};

/// \brief A message to be transmitted over the link.
///
/// The CRC will be calculated automatically.
struct Message {
    /// \brief The message's type.
    PacketType type;
    /// \brief The message's sequence number (valid: 0..7).
    uint8_t    sequenceNo;
    /// \brief The data to be sent.
    ///
    /// Byte-stuffing will be automatically added to sent messages and
    /// removed from received messages.
    QByteArray data;

    Message(PacketType t, const QByteArray &d) :
        type(t), data(d) {}
    Message() : type(PacketType::unknown), data{} {}
};

class Link : public QObject
{
    Q_OBJECT
private:
    /// \brief The QIODevice over which this Link communicates.
    QIODevice *port = nullptr;
    /// \brief A timer restarted when traffic is received; will trigger a
    /// timeout if more than 5000 ms pass without more traffic or a completed
    /// message.
    QTimer *readTimer = nullptr;
    /// \brief An internal buffer into which traffic to be sent is written.
    QBuffer writeBuf;
    /// \brief An internal buffer into which received traffic is written.
    QByteArray readBuf;
    /// \brief A QMutex that is locked iff data is being transmitted.
    QMutex busy;
    /// \brief The number of bytes remaining to write in the current packet.
    qint64 numBytesToWrite = 0;
    /// \brief The sequence number of the next packet to be sent; it's a
    /// uint64 here to eliminate an alignment warning.
    quint64 nextSeq = 0;
    bool validMessageReceived();
    /// \brief Read a message from the buffer.
    /// \param popCompleteMessage Iff true and a valid and complete
    /// message is in the buffer, clear the buffer afterwards.
    /// \return The message, or nullptr if none available.
    std::unique_ptr<Message> parseMessage(bool popCompleteMessage = true);
public:
    explicit Link(QObject *parent = nullptr);
    /// \brief Send the provided message to the device.
    /// \return true if we could start the send; false if the link
    /// was busy.
    bool send(const Message &msg);
    /// \brief Set the port that this Link should use.
    void setPort(QIODevice &port);
signals:
    /// \brief Emitted when a message has been received with a valid CRC.
    void packetReceived(Message);

public slots:
    /// \brief Notify this object that a scheduled write has been completed.
    void bytesWritten(qint64 numBytes);
    /// \brief Notify this object that bytes are available for reading on the port.
    void readyRead();
    /// \brief Notify this object that a read timeout has occured.
    void readTimeout();

friend class ::TestLink;
};
}
