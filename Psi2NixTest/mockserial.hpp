// Copyright © 2019 Brad Ackerman.
// Licensed under the MIT License (LICENSE.txt in this repository).

#pragma once

#include <QBuffer>
#include <QIODevice>
#include <QObject>

/// \brief A mock serial port to be used for testing.
class MockSerial : public QIODevice
{
    Q_OBJECT
public:
    explicit MockSerial(QObject *parent = nullptr);
    /// \brief A buffer containing the data sent by the program under test
    /// through this mock serial port.
    QBuffer sendBuf;
    /// \brief A buffer containing the data to be received by the program
    /// under test through this mock serial port.
    QBuffer recvBuf;
    /// \brief Add data to our buffer to be read by the program under test.
    void sendData(const QByteArray &data);
    /// \brief Add data to our buffer to be read by the program under test.
    void sendData(const char *data, qint64 size);
    /// \brief Return the number of bytes available to read.
    qint64 bytesAvailable() const override;
protected:
    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *data, qint64 maxSize) override;

signals:
    void bytesWritten(qint64 bytes);
    void readyRead();
};
