#ifndef OCFBINNARYSWITCHWITHBATTERY_H
#define OCFBINNARYSWITCHWITHBATTERY_H

#include <QObject>
#include "OicBaseDevice.h"

class OcfBinnarySwitchWithBattery: public OicBaseDevice
{
public:
    OcfBinnarySwitchWithBattery(QString name, QString id);
    void updateValue(quint16 pressed, quint16 battery);
};

#endif // OCFBINNARYSWITCHWITHBATTERY_H
