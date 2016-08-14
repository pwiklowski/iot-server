#ifndef OICBINNARYSWITCH_H
#define OICBINNARYSWITCH_H

#include <QObject>
#include "OicBaseDevice.h"

class OicBinnarySwitch : public OicBaseDevice
{
public:
    OicBinnarySwitch(QString name, QString id);
    void updateValue(bool pressed);
};

#endif // OICBINNARYSWITCH_H
