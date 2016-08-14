#ifndef OICBINNARYSWITCH_H
#define OICBINNARYSWITCH_H

#include <QObject>
#include "OicBaseDevice.h"

class OicBinnarySwitch : public OicBaseDevice
{
public:
    OicBinnarySwitch(QString name, QString id);
};

#endif // OICBINNARYSWITCH_H
