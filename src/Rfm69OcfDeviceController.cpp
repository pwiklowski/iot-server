#include "Rfm69OcfDeviceController.h"
#include "rfm69.hpp"

#define FREQUENCY RF69_868MHZ
#define NODEID 1
#define NETWORKID 100
#define TXPOWER 31
// A 16 bit password
#define CRYPTPASS "0123456789012345"


Rfm69OcfDeviceController::Rfm69OcfDeviceController(SmartHomeServer* s) :
    OcfDeviceController(s)
{

}

uint16_t Rfm69OcfDeviceController::readPacket(uint8_t* buf, uint16_t maxSize, String* address){

    return 0;
}

bool Rfm69OcfDeviceController::init(){
    RFM69 rfm69(false); // false = RFM69W, true = RFM69HW
    rfm69.reset();
    rfm69.init();
    rfm69.sleep();

    rfm69.setPowerDBm(0); // +10 dBm

    rfm69.setMode(RFM69_MODE_RX);
    rfm69.waitForModeReady();



    return true;
}

void Rfm69OcfDeviceController::send_packet(COAPPacket* packet){


}
