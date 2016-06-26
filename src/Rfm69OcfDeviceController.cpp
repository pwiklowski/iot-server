#include "Rfm69OcfDeviceController.h"
#include <unistd.h>

#define FREQUENCY RF69_868MHZ

Rfm69OcfDeviceController::Rfm69OcfDeviceController(SmartHomeServer* s) :
    OcfDeviceController(s)
{
    rfm69 = new Rfm69();
}

uint16_t Rfm69OcfDeviceController::readPacket(uint8_t* buf, uint16_t maxSize, String* address){
    int bytesReceived = rfm69->receivePacket(buf, maxSize);

    *address = "rfm69";

    if (bytesReceived >0){
       qDebug() << "bytes recieved" <<bytesReceived;
    }
	
    return bytesReceived;
}

bool Rfm69OcfDeviceController::init(){
    rfm69->reset();
    rfm69->init();
    rfm69->sleep();
    rfm69->setPowerDBm(0); // +10 dBm
    rfm69->setMode(RFM69_MODE_RX);
    rfm69->waitForModeReady();

    return true;
}

void Rfm69OcfDeviceController::send_packet(COAPPacket* packet){
    uint8_t buffer[1024];
    size_t response_len;
    packet->build(buffer, &response_len);
    rfm69->sendPacket(buffer, response_len);
}
