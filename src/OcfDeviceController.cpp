#include "OcfDeviceController.h"
#include "QDebug"
#include <linux/ip.h>
#include <netinet/igmp.h>
#include <poll.h>

OcfDeviceController::OcfDeviceController(SmartHomeServer* parent) : QObject((QObject*)parent)
{
    m_server = parent;

    QObject::connect(this, SIGNAL(deviceAdded(Device*)), m_server, SLOT(deviceAdded(Device*)));
    QObject::connect(this, SIGNAL(deviceRemoved(Device*)), m_server, SLOT(deviceRemoved(Device*)));


    m_client = new OICClient([&](COAPPacket* packet){
        this->send_packet(packet);
    });
    m_client->start("","");

    pthread_create(&m_thread, NULL, &OcfDeviceController::run, this);
    pthread_create(&m_deviceThread, NULL, &OcfDeviceController::waitForDevices, this);
    findDevices();


    m_timer.setInterval(5000);
    m_timer.setSingleShot(false);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(ping()));
    m_timer.start();

    m_deviceSearchTimer.setInterval(10*1000);

    m_deviceSearchTimer.setSingleShot(false);
    connect(&m_deviceSearchTimer, SIGNAL(timeout()), this, SLOT(findDevices()));
    m_deviceSearchTimer.start();
}

void* OcfDeviceController::waitForDevices(void* param){
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

void OcfDeviceController::ping(){

    for (Device* d: m_clientList){
        qDebug() << "ping" << d->getAddress();
        m_client->ping(d->getAddress().toLatin1().data(), [=](COAPPacket* p){
            if (p == 0){
                qDebug() << "Remove" << d->getName() << d->getID();
                emit deviceRemoved(d);
                m_clientList.removeOne(d);
            }else{
                qDebug() << "pong";
            }
        });
    }
}




bool OcfDeviceController::isDeviceOnList(QString id){
   for (Device* d: m_clientList){
       if (d->getID() == id){
           return true;
       }
   }
   return false;
}

void OcfDeviceController::findDevices()
{
    m_client->searchDevices([&](COAPPacket* packet){
        if (!packet) return false;
        cbor message;
        cbor::parse(&message, packet->getPayload());

        if (packet->getCode() != COAP_RSPCODE_CONTENT)
            return false;

        for (uint16_t i=0; i<message.toArray()->size(); i++){
            cbor device = message.toArray()->at(i);

            cbor naame = device.getMapValue("n");
            String name = device.getMapValue("n").toString();
            String di= device.getMapValue("di").toString();


            if (isDeviceOnList(QString(di.c_str()))) continue;

            qDebug() << "new device"<<  name.c_str() << di.c_str();
            cbor links = device.getMapValue("links");
            OICDevice* dev = new OICDevice(di, name, packet->getAddress(), m_client);

            for (uint16_t j=0; j< links.toArray()->size(); j++){
                cbor link = links.toArray()->at(j);


                String href = link.getMapValue("href").toString();
                String rt = link.getMapValue("rt").toString();
                String iff = link.getMapValue("if").toString();

                dev->getResources()->push_back(new OICDeviceResource(href, iff, rt, dev, m_client));
            }

            Device* d = new Device(dev, this);
            connect(d, SIGNAL(variablesChanged(QString,QString,QVariantMap)), m_server, SLOT(onValueChanged(QString, QString,QVariantMap)));
            m_clientList.append(d);
            emit deviceAdded(d);
        }
    });
}


String OcfDeviceController::convertAddress(sockaddr_in a){
    char addr[30];
    sprintf(addr, "%d.%d.%d.%d %d",
            (uint8_t) (a.sin_addr.s_addr),
            (uint8_t) (a.sin_addr.s_addr >> 8),
            (uint8_t) (a.sin_addr.s_addr >> 16 ),
            (uint8_t) (a.sin_addr.s_addr >> 24),
            htons(a.sin_port));

    return addr;
}
void* OcfDeviceController::run(void* param){
    OcfDeviceController* d = (OcfDeviceController*) param;
    OICClient* oic_server = d->getClient();
    COAPServer* coap_server = oic_server->getCoapServer();


    const int on = 1;
    int fd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    d->setSocketFd(fd);

    struct sockaddr_in serv,client;
    struct ip_mreq mreq;

    serv.sin_family = AF_INET;
    serv.sin_port = 0;
    serv.sin_addr.s_addr = htonl(INADDR_ANY);

    uint8_t buffer[1024];
    socklen_t l = sizeof(client);
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
        qDebug("Unable to set reuse");
        return 0;
    }
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000000;

    if( bind(fd, (struct sockaddr*)&serv, sizeof(serv) ) == -1)
    {
        qDebug("Unable to bind");
        return 0;
    }

    struct pollfd pfd;
    int res;

    pfd.fd = fd;
    pfd.events = POLLIN;

    size_t rc;
    while(1){
        rc = poll(&pfd, 1, 200); // 1000 ms timeout
        if (rc >0){
            rc = recvfrom(fd,buffer,sizeof(buffer),0,(struct sockaddr *)&client,&l);
            COAPPacket* p = COAPPacket::parse(buffer, rc, d->convertAddress(client).c_str());
            coap_server->handleMessage(p);
        }
        coap_server->tick();
    }
}
void OcfDeviceController::send_packet(COAPPacket* packet){
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
void OcfDeviceController::send_packet(sockaddr_in destination, COAPPacket* packet){

    uint8_t buffer[1024];
    size_t response_len;
    socklen_t l = sizeof(destination);
    packet->build(buffer, &response_len);

    sendto(m_socketFd, buffer, response_len, 0, (struct sockaddr*)&destination, l);
}
