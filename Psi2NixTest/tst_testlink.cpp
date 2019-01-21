// Copyright © 2019 Brad Ackerman.
// Licensed under the MIT License (LICENSE.txt in this repository).

#include <QtTest>
#include <QCoreApplication>

#include "link.hpp"

class TestLink : public QObject
{
    Q_OBJECT

private:
    std::unique_ptr<CommsLink::Link> link;
    std::unique_ptr<QBuffer> port;

private slots:
    void testSendData();
    void init();
};

void TestLink::init() {
    link = std::make_unique<CommsLink::Link>();
    port = std::make_unique<QBuffer>();
    port->open(QBuffer::ReadWrite);
    link->setPort(*port);
}

void TestLink::testSendData()
{
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
    link->send(CommsLink::PacketType::data, testCase);
    // the QBuffer completes the send synchronously.
    const QByteArray actual = port->data();
    QCOMPARE(actual, expected);
}

QTEST_MAIN(TestLink)

#include "tst_testlink.moc"
