#ifndef RFM69DEVICECONTROLLER_H
#define RFM69DEVICECONTROLLER_H

#include <QObject>
#include "devices/OicBaseDevice.h"
#include "rfm69.h"

typedef enum{
    OIC_R_SWITCH_BINARY


}OcfDeviceType;




class Rfm69DeviceController: public QObject
{
    Q_OBJECT
public:
    Rfm69DeviceController(QObject* object = 0);


    void start();
    static void* run(void* param);
signals:

public slots:
    void parseMessage(QByteArray message);


private:
    QString createDeviceId(quint32 id);
    OicBaseDevice* getDevice(quint32 id);

    QList<OicBaseDevice*>  m_clientList;
    bool isDeviceOnList(QString id);
    pthread_t m_thread;
};

#endif // RFM69DEVICECONTROLLER_H
