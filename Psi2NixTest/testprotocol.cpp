#include <QtTest>
#include <QSignalSpy>

#include "testprotocol.hpp"

void TestProtocol::init() {
    port = std::make_unique<MockSerial>();
    link = std::make_unique<CommsLink::Link>();
    link->setPort(*port);
    protocol = std::make_unique<CommsLink::Protocol>();
}

void TestProtocol::cleanup() {
    protocol.reset();
    link.reset();
    port.reset();
}

void TestProtocol::testLinkRequestSent() {
    // Verify that a connection request is sent automatically.
    qRegisterMetaType<MockSerial*>();
    QSignalSpy spy(&*port, &QIODevice::bytesWritten);
    QVERIFY(spy.isValid());
    // Connect to our link.
    protocol->setLink(*link);
    QVERIFY(spy.wait(1500));
    const quint8 expectedData[]{
        0x16, 0x10, 0x02,       // preamble
        0x01,                   // channel number
        0x10, 0x10,             // type/counter
        0x10, 0x03,              // end of packet
        0x00, 0x5c              // CRC
    };
    QByteArray expected(reinterpret_cast<const char *>(expectedData),
                        sizeof(expectedData));
    const QByteArray &actual = port->sendBuf.buffer();
    QCOMPARE(actual, expected);
}
