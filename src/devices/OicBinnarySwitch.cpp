#include "OicBinnarySwitch.h"
#include "cbor.h"
#include "OICResource.h"
#include "QDebug"


OicBinnarySwitch::OicBinnarySwitch(QString name, QString id) :
    OicBaseDevice(name, id)
{

    server = new OICServer(name.toLatin1().data(), id.toLatin1().data(), [&](COAPPacket* packet){
        this->send_packet(packet);
    });

    cbor* initial = new cbor(CBOR_TYPE_MAP);
    initial->append("rt", "oic.r.switch.binary");
    initial->append("value", 1);

    OICResource* button = new OICResource("Switch", "/switch", "oic.r.switch.binary","oic.if.r", [&](cbor* data){
        qDebug() << "switch updated";
    }, initial);
    server->addResource(button);

    start();
}

void OicBinnarySwitch::updateValue(bool pressed){
    notifyObservers("/switch", pressed ? 1 : 0);
}
