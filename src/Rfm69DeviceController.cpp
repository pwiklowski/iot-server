#include "Rfm69DeviceController.h"
#include "QDebug"
#include "OICDevice.h"
#include <unistd.h>
#include "devices/OicBinnarySwitch.h"



Rfm69DeviceController::Rfm69DeviceController(QObject* parent): QObject(parent)
{

}
void Rfm69DeviceController::start(){
    pthread_create(&m_thread, NULL, &Rfm69DeviceController::run, this);
}

void* Rfm69DeviceController::run(void* param){
    Rfm69DeviceController* ctrl = (Rfm69DeviceController*)param;

    uint8_t packet[512];
    int16_t packet_len;

    Rfm69* rfm69 = new Rfm69();
    rfm69->reset();
    bool res = rfm69->init();
    if (!res) return 0;

    rfm69->sleep();
    rfm69->setPowerDBm(0); // +10 dBm
    rfm69->setMode(RFM69_MODE_RX);
    rfm69->waitForModeReady();

    uint8_t ack;
    while(1){
        //packet_len = rfm69->receivePacket((uint8_t*)packet, 512);
        packet_len = rfm69->read(packet, 512, &ack);

        if (packet_len > 0){
            qDebug() << "Received something" << packet_len;
            QByteArray data((char*)packet, packet_len);
            QMetaObject::invokeMethod(ctrl, "parseMessage", Q_ARG(QByteArray, data));
        }
        usleep(1000*100);


    }
}
QString Rfm69DeviceController::createDeviceId(quint32 id){
    return QString("00000000-0000-0000-FFFF-0000%1").arg(id, 8, 16, QChar('0')).toUpper();
}


OicBaseDevice* Rfm69DeviceController::getDevice(quint32 id){
    QString uuid = createDeviceId(id);

    for (OicBaseDevice* dev: m_clientList){
        if (dev->getId() == uuid){
            return dev;
        }
    }
    return 0;
}


void Rfm69DeviceController::parseMessage(QByteArray message){
    quint32 deviceId = (message.at(3) << 24) | (message.at(2) << 16) | (message.at(1) << 8) | message.at(0);
    OcfDeviceType type = (OcfDeviceType)message.at(4);
    OicBaseDevice* device = getDevice(deviceId);

    if (device == 0){
        qDebug() << "Create new Rfm69 device" << createDeviceId(deviceId);
        device = new OicBinnarySwitch("Virtual Button", createDeviceId(deviceId));
        m_clientList.append(device);
    }

    if (type == OIC_R_SWITCH_BINARY){
        quint8 value = message.at(5);
        qDebug() << createDeviceId(deviceId) << "value" << value;
        ((OicBinnarySwitch*)device)->updateValue(value);
    }


}

