// Copyright © 2019 Brad Ackerman.
// Licensed under the MIT License (LICENSE.txt in this repository).

#include "protocol.hpp"

#include <QMutexLocker>
#include <QtDebug>

namespace CommsLink {

const std::chrono::milliseconds connRequestInterval{1000};

const Message connRequest{ /*!< Connection request packet */
    PacketType::linkRequest,
    QByteArray{}
};

Protocol::Protocol(QObject *parent) : QObject(parent)
{
    connect(&requestTimer, &QTimer::timeout,
            this, &Protocol::timeForRequest);
    requestTimer.start(connRequestInterval);
}

void Protocol::setLink(Link &aLink) {
    myLink = &aLink;
}

bool Protocol::isConnected() {
    return connected;
}

void Protocol::timeForRequest() {
    QMutexLocker lock(&sendQueueMutex);
    if (!connected && (myLink != nullptr)
            && sendQueue.isEmpty()) {
        bool success = myLink->send(connRequest);
        if (!success) {
            qDebug() << "Tried to send conn request but link busy";
        }
    }
}

}
