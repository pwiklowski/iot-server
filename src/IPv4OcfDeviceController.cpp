#include "IPv4OcfDeviceController.h"

IPv4OcfDeviceController::IPv4OcfDeviceController(SmartHomeServer *server):
    OcfDeviceController(server)
{

    pthread_create(&m_deviceThread, NULL, &IPv4OcfDeviceController::waitForDevices, this);

}


String IPv4OcfDeviceController::convertAddress(sockaddr_in a){
    char addr[30];
    sprintf(addr, "%d.%d.%d.%d %d",
            (uint8_t) (a.sin_addr.s_addr),
            (uint8_t) (a.sin_addr.s_addr >> 8),
            (uint8_t) (a.sin_addr.s_addr >> 16 ),
            (uint8_t) (a.sin_addr.s_addr >> 24),
            htons(a.sin_port));

    return addr;
}


void IPv4OcfDeviceController::send_packet(COAPPacket* packet){
    String destination = packet->getAddress();
    size_t pos = destination.find(" ");
    String ip = destination.substr(0, pos);
    uint16_t port = atoi(destination.substr(pos).c_str());

    struct sockaddr_in client;

    client.sin_family = AF_INET;
    client.sin_port = htons(port);
    client.sin_addr.s_addr = inet_addr(ip.c_str());

    send_packet(client, packet);
}
void IPv4OcfDeviceController::send_packet(sockaddr_in destination, COAPPacket* packet){

    uint8_t buffer[1024];
    size_t response_len;
    socklen_t l = sizeof(destination);
    packet->build(buffer, &response_len);

    sendto(m_socketFd, buffer, response_len, 0, (struct sockaddr*)&destination, l);
}

void* IPv4OcfDeviceController::waitForDevices(void* param){
    OcfDeviceController* d = (OcfDeviceController*) param;

    const int on = 1;
    int fd = socket(AF_INET,SOCK_RAW,IPPROTO_IGMP);
    if (fd <0){
        qDebug() << "error opening IGMP socket";
        return 0;
    }

    uint8_t buffer[1024];

    while(1){
        int rc = recv(fd,buffer,sizeof(buffer),0);

        if (rc>0){
            iphdr *iph = (iphdr*)buffer;
            uint8_t ip_hdr_len = iph->ihl * 4;
            igmp* igmpp = (igmp*) (buffer + ip_hdr_len);

            if (igmpp->igmp_type == IGMP_V1_MEMBERSHIP_REPORT ||
                igmpp->igmp_type == IGMP_V2_MEMBERSHIP_REPORT ||
                igmpp->igmp_type == IGMP_V2_LEAVE_GROUP){

                if (igmpp->igmp_group.s_addr == inet_addr("224.0.1.187")){
                    d->findDevices();
                }
            }
        }
    }
}


bool IPv4OcfDeviceController::init(){
    const int on = 1;
    m_socketFd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);

    struct sockaddr_in serv;
    struct ip_mreq mreq;

    serv.sin_family = AF_INET;
    serv.sin_port = 0;
    serv.sin_addr.s_addr = htonl(INADDR_ANY);

    if(setsockopt(m_socketFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
        qDebug("Unable to set reuse");
        return false;
    }
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000000;

    if( bind(m_socketFd, (struct sockaddr*)&serv, sizeof(serv) ) == -1)
    {
        qDebug("Unable to bind");
        return false;
    }

    return true;
}


uint16_t IPv4OcfDeviceController::readPacket(uint8_t* buf, uint16_t maxSize, String* address){
    struct pollfd pfd;
    int res = sizeof(*buf);

    pfd.fd = m_socketFd;
    pfd.events = POLLIN;

    struct sockaddr_in client;
    socklen_t l = sizeof(client);
    size_t rc = poll(&pfd, 1, 200); // 1000 ms timeout
    if (rc >0){
        rc = recvfrom(m_socketFd, buf, maxSize, 0, (struct sockaddr *)&client,&l);

        *address = convertAddress(client);
    }
    return rc;
}
