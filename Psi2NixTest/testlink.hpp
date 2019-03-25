#pragma once

#include <memory>
#include <QObject>

#include "link.hpp"
#include "mockserial.hpp"

class TestLink : public QObject
{
    Q_OBJECT

private:
    std::unique_ptr<CommsLink::Link> link;
    std::unique_ptr<MockSerial> port;
    std::unique_ptr<CommsLink::Message> receivedMsg;
private slots:
    void testSendData();
    void testSendLinkRequest();
    void testReceiveData();
    void testReceiveMultipleReads();
    void init();
    void receiveMessage(CommsLink::Message);
};
