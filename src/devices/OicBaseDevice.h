#ifndef OICBASEDEVICE_H
#define OICBASEDEVICE_H

#include <QObject>
#include "OICServer.h"
#include <arpa/inet.h>
#include <net/if.h>

class OicBaseDevice : public QObject
{
    Q_OBJECT
public:
    OicBaseDevice();
    OICServer* getServer(){return server;}
    void setSocketFd(int s) { m_socketFd = s;}

    void start();
signals:

public slots:
    void notifyObservers(QString name, quint8 value);
    static void* runDiscovery(void* param);
    static void* run(void* param);

protected:
    OICServer* server;
    QString m_name;
    QString m_id;
    void send_packet(COAPPacket* packet);
private:

    String convertAddress(sockaddr_in a);
    void send_packet(sockaddr_in destination, COAPPacket* packet);

    int m_socketFd;
    cbor* m_value;
    pthread_t m_thread;
    pthread_t m_discoveryThread;


};

#endif // OICBASEDEVICE_H
