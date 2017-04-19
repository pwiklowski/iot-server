#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include "QVariant"
#include "QUrl"
#include "cbor.h"
#include "OICDevice.h"
#include "OICDeviceResource.h"


class SmartHomeServer;
class Device;

class DeviceVariable: public QObject
{
    Q_OBJECT
public:
    DeviceVariable(OICDeviceResource* res, OICDevice *dev);

signals:
    void valueChanged(QString id, QString path, QVariantMap val);

public slots:
    void observe();
    void set(QVariantMap value);

    QString getInterface() { return m_resource->getInterface().c_str();}
    QString getResourceType() { return m_resource->getResourceType().c_str();}
    QString getHref() { return m_resource->getHref().c_str();}
    QString getName() { return m_resource->getName().c_str();}

private:
    void post(QVariantMap value);
    void unobserve();

    QVariantMap toQMap(cbor* map);
    OICDeviceResource* m_resource;
    OICDevice* m_device;
    QString m_id;
};

class Device : public QObject
{
    Q_OBJECT
public:
    explicit Device(OICDevice* dev, QObject *parent);

    QString getAddress() { return QString(m_device->getAddress().c_str());}
    QString getID() { return m_device->getId().c_str();}
    QString getName() { return m_device->getName().c_str();}

    QList<DeviceVariable*>* getVariables(){ return &m_variables; }

    DeviceVariable* getVariable(QString resource);
signals:
    void variablesChanged(QString id, QString path, QVariantMap val);
    void setVariableValue(QString url, qint32 value);

private:
    OICDevice* m_device;

    QList<DeviceVariable*> m_variables;

};

#endif // DEVICE_H
