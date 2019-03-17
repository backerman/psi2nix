#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include <QObject>

class Protocol : public QObject
{
    Q_OBJECT
public:
    explicit Protocol(QObject *parent = nullptr);

signals:

public slots:
};

#endif // PROTOCOL_HPP