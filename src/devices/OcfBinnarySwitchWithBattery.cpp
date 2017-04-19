#include "OcfBinnarySwitchWithBattery.h"
#include "QDebug"

OcfBinnarySwitchWithBattery::OcfBinnarySwitchWithBattery(QString name, QString id):OicBaseDevice(name, id)
{
    server = new OICServer(name.toLatin1().data(), id.toLatin1().data(), [&](COAPPacket* packet){
        this->send_packet(packet);
    });

    cbor* initial = new cbor(CBOR_TYPE_MAP);
    initial->append("rt", "oic.r.switch.binary");
    initial->append("value", 1);

    cbor* initialBattery = new cbor(CBOR_TYPE_MAP);
    initialBattery->append("rt", "oic.r.energy.battery");
    initialBattery->append("charge", 100);

    OICResource* button = new OICResource("Switch", "/switch", "oic.r.switch.binary","oic.if.r", [&](cbor* data){
        qDebug() << "switch updated";
    }, initial);
    server->addResource(button);


    OICResource* battery = new OICResource("Battery level", "/battery", "oic.r.energy.battery","oic.if.r", [&](cbor* data){
        qDebug() << "battery updated";
    }, initialBattery);
    server->addResource(battery);

    start();
}


void OcfBinnarySwitchWithBattery::updateValue(quint16 pressed, quint16 battery){
    notifyObservers("/switch", pressed ? 1 : 0);


    cbor value(CBOR_TYPE_MAP);
    value.append("rt", "oic.r.energy.battery");
    value.append("charge", battery);

    Vector<uint8_t> data;
    value.dump(&data);

    server->notify("/battery", &data);
}
