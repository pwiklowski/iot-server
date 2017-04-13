#include "nrf24lDeviceController.h"
#include "QDebug"
#include "QMutexLocker"


#define MINIMAL_VOLTAGE 2.0
#define MAX_VOLTAGE 3.3

#define CHARGE_STEP ((MAX_VOLTAGE-MINIMAL_VOLTAGE)/100)

nrf24lDeviceController::nrf24lDeviceController()
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

    bool res = m_serial->open(QIODevice::ReadWrite);
    qDebug() << "Serial port opened" << res;
    m_serial->setBaudRate(QSerialPort::Baud57600);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);


   connect(m_serial, SIGNAL(readyRead()), this, SLOT(read()));
}

QString nrf24lDeviceController::createDeviceId(quint16 id){
    return QString("00000000-0000-0002-FFFF-%1").arg(id, 8, 16, QChar('0')).toUpper();
}


OicBaseDevice* nrf24lDeviceController::getDevice(quint16 id){
    QString uuid = createDeviceId(id);

    for (OicBaseDevice* dev: m_clientList){
        if (dev->getId() == uuid){
            return dev;
        }
    }
    return 0;
}

void nrf24lDeviceController::read(){
    if (m_serial->isOpen())
    {
        while(m_serial->canReadLine()){
            QByteArray data = m_serial->readLine();
            parseMessage(data);
        }
    }
}

void nrf24lDeviceController::parseMessage(QString data)
{
    QStringList params = data.split(" ");
    qDebug() << params;

    quint16 deviceId = params.at(1).toInt();
    double batteryVolts = (((float)params.at(2).toInt())/255)*3*1.2;

    qint16 batteryChageLevel = (int)((batteryVolts-MINIMAL_VOLTAGE)/CHARGE_STEP);


    qDebug() << "battery volts" << batteryChageLevel << CHARGE_STEP <<  (batteryVolts-MINIMAL_VOLTAGE)/CHARGE_STEP;

    if(batteryChageLevel > 100) batteryChageLevel = 100;
    if (batteryChageLevel < 0) batteryChageLevel = 0;

    quint16 eventValue = params.at(4).toInt();

    OicBaseDevice* device = getDevice(deviceId);

    QString devId = createDeviceId(deviceId);

    if (device == 0){
        qDebug() << "Create new nrf24 device" << devId;
        device = new OcfBinnarySwitchWithBattery("NRF24 Button " + deviceId, devId);
        m_clientList.append(device);
    }

    ((OcfBinnarySwitchWithBattery*)device)->updateValue(eventValue, batteryChageLevel);
}
