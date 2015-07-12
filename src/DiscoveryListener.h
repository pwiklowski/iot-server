#ifndef DISCOVERYLISTENER_H
#define DISCOVERYLISTENER_H

#include <QObject>
#include "QUdpSocket"

class DiscoveryListener : public QObject
{
    Q_OBJECT
public:
    explicit DiscoveryListener(QObject *parent = 0);

signals:

public slots:
    void readDatagram();
private:
    QUdpSocket* m_socket;

};

#endif // DISCOVERYLISTENER_H
