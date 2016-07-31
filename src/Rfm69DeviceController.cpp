#include "Rfm69DeviceController.h"
#include "QDebug"

Rfm69DeviceController::Rfm69DeviceController(): QObject(0)
{
}
void Rfm69DeviceController::start(){
    pthread_create(&m_thread, NULL, &Rfm69DeviceController::run, this);
}

void* Rfm69DeviceController::run(void* param){
    uint8_t packet[512];
    uint16_t packet_len;

    Rfm69* rfm69 = new Rfm69();
    rfm69->reset();
    rfm69->init();
    rfm69->sleep();
    rfm69->setPowerDBm(0); // +10 dBm
    rfm69->setMode(RFM69_MODE_RX);
    rfm69->waitForModeReady();

    while(1){
        packet_len = rfm69->receivePacket((uint8_t*)packet, 512);

        if (packet_len > 0){
            qDebug() << "Received something" << packet_len;
        }

    }
}

