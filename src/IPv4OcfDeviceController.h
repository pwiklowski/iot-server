#ifndef IPV4OCFDEVICECONTROLLER_H
#define IPV4OCFDEVICECONTROLLER_H

#include "OcfDeviceController.h"
#include <linux/ip.h>
#include <netinet/igmp.h>
#include <poll.h>
#include <arpa/inet.h>
#include <net/if.h>

class IPv4OcfDeviceController : public OcfDeviceController
{
public:
    IPv4OcfDeviceController(SmartHomeServer* server);

    static void* waitForDevices(void* param);
    void setSocketFd(int s) { m_socketFd = s;}
signals:

public slots:


protected:
    void send_packet(COAPPacket* packet) override ;
    uint16_t readPacket(uint8_t* buf, uint16_t maxSize, String* address) override;
    bool init() override;

    String convertAddress(sockaddr_in a);
    void send_packet(sockaddr_in destination, COAPPacket* packet);
    int m_socketFd;
    pthread_t m_deviceThread;

private:
};

#endif // IPV4OCFDEVICECONTROLLER_H
