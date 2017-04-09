#ifndef NRF24LDEVICECONTROLLER_H
#define NRF24LDEVICECONTROLLER_H

#include "QSerialPort"
#include "QSerialPortInfo"
#include "devices/OcfBinnarySwitchWithBattery.h"
#include "QMutex"

class nrf24lDeviceController  : public QObject
{
    Q_OBJECT
public:
    nrf24lDeviceController();
public slots:
    void read();
    void parseMessage(QString data);
private:
    QString createDeviceId(quint16 id);
    OicBaseDevice* getDevice(quint16 id);
    void parseEvent(quint16 addr, QByteArray data);
    QSerialPort* m_serial;
    QByteArray m_buffer;
    QList<OicBaseDevice*>  m_clientList;
    QMutex mutex;
};

#endif // NRF24LDEVICECONTROLLER_H
