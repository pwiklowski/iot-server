#include "SerialBleScanner.h"
#include "QDebug"
#include "QTimer"
#include "QMutexLocker"
#include "QSerialPort"
#include "QSerialPortInfo"

SerialBleScanner::SerialBleScanner(QObject* parent):
    QObject(parent)
{
    QList<QSerialPortInfo> infos = QSerialPortInfo::availablePorts();

    foreach (QSerialPortInfo info, infos)
    {
        qDebug() << info.portName();
    }


    m_serial = new QSerialPort(infos.at(0), this);
    bool res = m_serial->open(QIODevice::ReadWrite);
    qDebug() << res;
    m_serial->setBaudRate(QSerialPort::Baud115200);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);
}


void SerialBleScanner::read()
{
    QMutexLocker locker(&mutex);
    if (m_serial->isOpen())
    {
        m_serial->waitForReadyRead(1000);
        QByteArray data = m_serial->readAll();

        m_buffer.append(data);
    }
    QTimer::singleShot(20,this, SLOT(read()));

    QMetaObject::invokeMethod(this, "parseMessage", Qt::QueuedConnection);
}


void SerialBleScanner::parseMessage()
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
    if (!address.startsWith("20"))
        qDebug() << address << data.toHex();

    if (len != data.size())
    {
        qWarning() << "WRONG DATA LENGTH" << len <<data.size();
    }
    else
    {
        emit iotEventReceived(address, data);
}



    m_buffer.remove(0, 10+len);




    if (m_buffer.size() >0)
         QMetaObject::invokeMethod(this, "parseMessage", Qt::QueuedConnection);
}
