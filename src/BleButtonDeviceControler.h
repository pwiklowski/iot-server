#ifndef BLEBUTTONDEVICECONTROLER_H
#define BLEBUTTONDEVICECONTROLER_H

#include "QtSerialPort/QSerialPort"
#include "QMutex"
#include "devices/OicBinnarySwitch.h"


class BleButtonDeviceControler : public QObject
{
    Q_OBJECT
public:
    BleButtonDeviceControler();

public slots:
    void read();
    void parseMessage();
private:
    QString createDeviceId(QString id);
    OicBaseDevice* getDevice(QString id);
    void parseEvent(QString addr, QByteArray data);

    QMutex mutex;
    QSerialPort* m_serial;
    QByteArray m_buffer;
    QList<OicBaseDevice*>  m_clientList;
    quint8 lastEvent;
};

#endif // BLEBUTTONDEVICECONTROLER_H
