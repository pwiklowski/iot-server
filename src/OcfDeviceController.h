#ifndef OCFDEVICECONTROLLER_H
#define OCFDEVICECONTROLLER_H

#include <QObject>
#include "QTimer"
#include "OICClient.h"
#include "IotDevice.h"
#include "Device.h"

class OcfDeviceController : public QObject
{
    Q_OBJECT
public:
    OcfDeviceController(QObject *parent);

    OICClient* getClient(){return m_client;}
    static void* run(void* param);

    void start();
signals:
    void deviceAdded(IotDevice* d);
    void deviceRemoved(IotDevice* d);
public slots:
    void findDevices();
    void ping();

protected:
private:
    QList<Device*>  m_clientList;
    QTimer m_timer;
    QTimer m_deviceSearchTimer;
    bool isDeviceOnList(QString id);


    virtual int readPacket(uint8_t* buf, uint16_t maxSize, String* address) = 0;
    virtual bool init() = 0;
    virtual void send_packet(COAPPacket* packet) = 0;

    OICClient* m_client;
    pthread_t m_thread;
    QObject* m_parent;

};

#endif // OCFDEVICECONTROLLER_H
