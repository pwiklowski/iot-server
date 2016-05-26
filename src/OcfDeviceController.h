#ifndef OCFDEVICECONTROLLER_H
#define OCFDEVICECONTROLLER_H

#include <QObject>
#include <arpa/inet.h>
#include <net/if.h>
#include "Device.h"
#include "QTimer"
#include "SmartHomeServer.h"

class OcfDeviceController : public QObject
{
    Q_OBJECT
public:
    explicit OcfDeviceController(SmartHomeServer *parent = 0);
    static void* run(void* param);

      static void* waitForDevices(void* param);
    OICClient* getClient(){return m_client;}
    void setSocketFd(int s) { m_socketFd = s;}
signals:
    void deviceAdded(IotDevice* d);
    void deviceRemoved(IotDevice* d);
public slots:
    void findDevices();
    void ping();


private:
    SmartHomeServer* m_server;
    QList<Device*>  m_clientList;
    QTimer m_timer;
    QTimer m_deviceSearchTimer;
    String convertAddress(sockaddr_in a);
    bool isDeviceOnList(QString id);


    void send_packet(sockaddr_in destination, COAPPacket* packet);
    void send_packet(COAPPacket* packet);
    OICClient* m_client;
    QObject* m_parent;
    pthread_t m_thread;
    pthread_t m_deviceThread;
    int m_socketFd;

};

#endif // OCFDEVICECONTROLLER_H
