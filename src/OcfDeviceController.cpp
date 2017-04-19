#include "OcfDeviceController.h"
#include "QDebug"



extern uint64_t get_current_ms();

OcfDeviceController::OcfDeviceController(SmartHomeServer* parent) : QObject((QObject*)parent)
{
    m_server = parent;

    QObject::connect(this, SIGNAL(deviceAdded(Device*)), m_server, SLOT(deviceAdded(Device*)));
    QObject::connect(this, SIGNAL(deviceRemoved(Device*)), m_server, SLOT(deviceRemoved(Device*)));


    m_client = new OICClient([&](COAPPacket* packet){
        this->send_packet(packet);
    });
    m_client->start("","");



    m_timer.setInterval(5000);
    m_timer.setSingleShot(false);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(ping()));
    m_timer.start();

    m_deviceSearchTimer.setInterval(10*1000);

    m_deviceSearchTimer.setSingleShot(false);
    connect(&m_deviceSearchTimer, SIGNAL(timeout()), this, SLOT(findDevices()));
    m_deviceSearchTimer.start();
}

void OcfDeviceController::start(){
    pthread_create(&m_thread, NULL, &OcfDeviceController::run, this);
    findDevices();
}

void OcfDeviceController::ping(){
    for (Device* d: m_clientList){
        //qDebug() << "ping" << d->getAddress();
        m_client->ping(d->getAddress().toLatin1().data(), [=](COAPPacket* p){
            if (p == 0){
                qDebug() << "Remove" << d->getName() << d->getID();
                emit deviceRemoved(d);
                m_clientList.removeOne(d);
            }else{
                //qDebug() << "pong";
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

        if (packet->getPayload()->size() == 0) return false;

        cbor::parse(&message, packet->getPayload());

        if (packet->getCode() != COAP_RSPCODE_CONTENT)
            return false;

        if (!message.is_array())
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
                String n = "empty";

                if (link.toMap()->has("n"))
                    n = link.getMapValue("n").toString();

                dev->getResources()->push_back(new OICDeviceResource(n, href, iff, rt, dev, m_client));
            }

            Device* d = new Device(dev, this);
            connect(d, SIGNAL(variablesChanged(QString,QString,QVariantMap)), m_server, SLOT(onValueChanged(QString, QString,QVariantMap)));
            m_clientList.append(d);
            emit deviceAdded(d);
        }
    });
}

void* OcfDeviceController::run(void* param){
    uint8_t buffer[1024];
    OcfDeviceController* d = (OcfDeviceController*) param;
    OICClient* oic_server = d->getClient();

    bool res = d->init();

    size_t rc;
    String address;

    uint64_t lastTick = get_current_ms();

    while(1){
        rc = d->readPacket(buffer, sizeof(buffer), &address);
        if (rc >0){
            COAPPacket* p = COAPPacket::parse(buffer, rc, address.c_str());
            if (p != 0){
                oic_server->handleMessage(p);
                delete p;
            }
        }
        oic_server->sendQueuedPackets();
        if ((get_current_ms() - lastTick) > 50){
            lastTick = get_current_ms();
            oic_server->checkPackets();
        }
    }
}

