#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include "QVariant"
#include "QUrl"
#include "cbor.h"
#include "OICDevice.h"
#include "OICDeviceResource.h"
#include "IotDevice.h"


class SmartHomeServer;
class Device;

class DeviceVariable: public IotDeviceVariable
{
    Q_OBJECT
public:
    DeviceVariable(OICDeviceResource* res, OICDevice *dev);

signals:
    void valueChanged(QString id, QString path, QVariantMap val);

public slots:
    void observe();
    void set(QVariantMap value);

private:
    void post(QVariantMap value);
    void unobserve();

    QVariantMap toQMap(cbor* map);
    OICDeviceResource* m_resource;
    OICDevice* m_device;
    QString m_id;
};

class Device : public IotDevice
{
    Q_OBJECT
public:
    explicit Device(OICDevice* dev, QObject *parent);

    QString getAddress() { return QString(m_device->getAddress().c_str());}
signals:
    void variablesChanged(QString id, QString path, QVariantMap val);
    void setVariableValue(QString url, qint32 value);

private:
    OICDevice* m_device;

};

#endif // DEVICE_H
