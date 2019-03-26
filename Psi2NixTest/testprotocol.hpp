#pragma once

#include <memory>
#include <QObject>

#include "protocol.hpp"
#include "mockserial.hpp"

class TestProtocol : public QObject
{
    Q_OBJECT
private:
    std::unique_ptr<MockSerial> port;
    std::unique_ptr<CommsLink::Link> link;
    std::unique_ptr<CommsLink::Protocol> protocol;

private slots:
    void init();
    void cleanup();
    void testLinkRequestSent();
};
