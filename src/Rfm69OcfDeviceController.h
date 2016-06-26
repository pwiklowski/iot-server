#ifndef RFM69OCFDEVICECONTROLLER_H
#define RFM69OCFDEVICECONTROLLER_H

#include "OcfDeviceController.h"
#include "rfm69.h"


class Rfm69OcfDeviceController : public OcfDeviceController
{
public:
    Rfm69OcfDeviceController(SmartHomeServer* s);
signals:

public slots:
private:
    int readPacket(uint8_t* buf, uint16_t maxSize, String* address) override;
    bool init() override;
    void send_packet(COAPPacket* packet) override;

    Rfm69* rfm69;

};

#endif // RFM69OCFDEVICECONTROLLER_H
