// Copyright © 2019 Brad Ackerman.
// Licensed under the MIT License (LICENSE.txt in this repository).

#pragma once

#include <QObject>
#include <QMutex>
#include <QTimer>

#include "link.hpp"

namespace CommsLink {

enum struct Layer {
    file,
    cthulhu
};

class Protocol : public QObject
{
    Q_OBJECT
public:
    explicit Protocol(QObject *parent = nullptr);
    /*!
     \brief Set this protocol's corresponding link object.

     \param[in] aLink The link to use.
    */
    void setLink(Link &aLink);
    /*!
     \brief Return whether this Protocol is connected to a remote endpoint.

     \return true iff connected.
    */
    bool isConnected();
signals:

private slots:
    /*!
     \brief Slot that receives a once-per-second timer
     to emit a connection request.
    */
    void timeForRequest();

private:
    /*! sends conn request iff disconnected */
    QTimer requestTimer;
    QMutex sendQueueMutex;
    QList<QByteArray> sendQueue;
    Link *myLink = nullptr;
    bool connected = false;
};
}
