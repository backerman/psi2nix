// Copyright © 2019 Brad Ackerman.
// Licensed under the MIT License (LICENSE.txt in this repository).

#include <QtTest>
#include <QCoreApplication>
#include <QSignalSpy>

#include "link.hpp"
#include "mockserial.hpp"

class TestLink : public QObject
{
    Q_OBJECT

private:
    std::unique_ptr<CommsLink::Link> link;
    std::unique_ptr<MockSerial> port;

private slots:
    void testSendData();
    void testSendLinkRequest();
    void testReceiveData();
    void init();
};

void TestLink::init() {
    link = std::make_unique<CommsLink::Link>();
    port = std::make_unique<MockSerial>();
    link->setPort(*port);
}

void TestLink::testSendData()
{
    qRegisterMetaType<MockSerial*>();
    QSignalSpy spy(&(*port), &QIODevice::bytesWritten);
    QVERIFY(spy.isValid());
    const char testCaseData[] = "FILE";
    // -1 here because we don't send the terminal \0.
    const QByteArray testCase(testCaseData, sizeof(testCaseData) - 1);
    const quint8 expectedData[]{
        0x16, 0x10, 0x02,       // preamble
        0x01,                   // channel number
        0x19,                   // type 3, seq 1
        0x46, 0x49, 0x4C, 0x45, // "FILE"
        0x10, 0x03,             // postamble
        0x2D, 0xBE              // CRC
    };
    const QByteArray expected(reinterpret_cast<const char *>(expectedData),
                              sizeof(expectedData));
    bool success = link->send(CommsLink::PacketType::data, testCase);
    QVERIFY(success);
    QVERIFY(spy.wait(250));
    const QByteArray &actual = port->sendBuf.buffer();
    QCOMPARE(actual, expected);
}

void TestLink::testSendLinkRequest() {
    QSignalSpy spy(&*port, &QIODevice::bytesWritten);
    const quint8 expectedData[]{
        0x16, 0x10, 0x02,       // preamble
        0x01,                   // channel number
        0x10, 0x10,             // type 1, seq 0
        0x10, 0x03,             // postamble
        0x00, 0x5C              // CRC
    };
    link->send(CommsLink::PacketType::linkRequest, QByteArray());
    const QByteArray expected(reinterpret_cast<const char *>(expectedData),
                              sizeof(expectedData));
    QVERIFY(spy.wait(250));
    const QByteArray &actual = port->sendBuf.buffer();
    QCOMPARE(actual, expected);
}

void TestLink::testReceiveData() {
    QSignalSpy spy(&(*port), &QIODevice::readyRead);
    QVERIFY(spy.isValid());
    const quint8 goodData[]{
        0x16, 0x10, 0x02,       // preamble
        0x01,                   // channel number
        0x10, 0x10,             // type 2, seq 0
        0x10, 0x03,             // postamble
        0x00, 0x5C              // CRC
    };
    port->sendData(reinterpret_cast<const char *>(goodData),
                         sizeof(goodData));
    QVERIFY(spy.wait(2500));
    std::unique_ptr<CommsLink::Message> msg = link->parseMessage();
    QVERIFY(msg);
    QVERIFY(msg->type == CommsLink::PacketType::linkRequest);
    QVERIFY(msg->sequenceNo == 0);
    QVERIFY(msg->data.size() == 0);
    QVERIFY(link->readBuf.size() == 0);
}

QTEST_MAIN(TestLink)

#include "tst_testlink.moc"
