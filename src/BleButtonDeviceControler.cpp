#include "BleButtonDeviceControler.h"
#include "QDebug"
#include "QTimer"
#include "QMutexLocker"
#include "QSerialPort"
#include "QSerialPortInfo"
#include "Settings.h"


BleButtonDeviceControler::BleButtonDeviceControler()
{
    Settings settings;

    if (settings.getValue("port").toString().isEmpty())
    {
        QList<QSerialPortInfo> infos = QSerialPortInfo::availablePorts();
        QSerialPortInfo port;
        foreach (QSerialPortInfo info, infos)
        {
            qDebug() << info.portName();
            if (info.portName().contains("USB"))
            {
                port = info;
                break;
            }
        }

        m_serial = new QSerialPort(port, this);
    }
    else
    {
        m_serial = new QSerialPort(settings.getValue("port").toString(), this);
    }

    bool res = m_serial->open(QIODevice::ReadWrite);
    qDebug() << "Serial port opened" << res;
    m_serial->setBaudRate(QSerialPort::Baud115200);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);


   connect(m_serial, SIGNAL(readyRead()), this, SLOT(read()));
}


void BleButtonDeviceControler::read()
{
    QMutexLocker locker(&mutex);
    if (m_serial->isOpen())
    {
        QByteArray data = m_serial->readAll();
        m_buffer.append(data);
    }

    if (m_buffer.size() >0)
        QMetaObject::invokeMethod(this, "parseMessage", Qt::QueuedConnection);
}

QString BleButtonDeviceControler::createDeviceId(QString id){
    return QString("00000000-0000-0001-FFFF-%1").arg(id).toUpper();
}


OicBaseDevice* BleButtonDeviceControler::getDevice(QString id){
    QString uuid = createDeviceId(id);

    for (OicBaseDevice* dev: m_clientList){
        if (dev->getId() == uuid){
            return dev;
        }
    }
    return 0;
}


void BleButtonDeviceControler::parseMessage()
{
    QMutexLocker locker(&mutex);
    int index = m_buffer.indexOf("ADV");

    if (index <0)
        return;

    if (index >0)
        m_buffer.remove(0, index);
    if (m_buffer.size() <=10)
        return;


    QString address = m_buffer.mid(3,6).toHex();
    quint8 len = m_buffer.at(9);
    if (m_buffer.size() <(10+len))
        return;


    QByteArray data = m_buffer.mid(10,len);

    if (len != data.size())
    {
        qWarning() << "WRONG DATA LENGTH" << len <<data.size();
    }
    else
    {
        quint8 messageId = data.at(3);

        if (messageId != lastEvent){ // TODO: make sure that last message id is for specific address
            lastEvent = messageId;
            parseEvent(address, data);
        }
    }



    m_buffer.remove(0, 10+len);

    if (m_buffer.size() >0)
         QMetaObject::invokeMethod(this, "parseMessage", Qt::QueuedConnection);
}



void BleButtonDeviceControler::parseEvent(QString addr, QByteArray data){

    OicBaseDevice* device = getDevice(addr);

    QString devId = createDeviceId(addr);

    if (device == 0){
        qDebug() << "Create new ble device" << devId;
        device = new OicBinnarySwitch("BLE Button " + addr, devId);
        m_clientList.append(device);
    }

    ((OicBinnarySwitch*)device)->updateValue(true);

}
