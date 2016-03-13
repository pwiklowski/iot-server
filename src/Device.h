#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include "QVariant"
#include "QUrl"
#include "cbor.h"
#include "OICDevice.h"
#include "OICDeviceResource.h"
#include "QVariant"

class SmartHomeServer;
class Device;

class DeviceVariable: public QObject
{
    Q_OBJECT
    Q_ENUMS(Status)
    Q_PROPERTY(QString iff READ getInterface CONSTANT)
    Q_PROPERTY(QString resourceType READ getResourceType CONSTANT)
    Q_PROPERTY(QString href READ getHref CONSTANT)
    Q_PROPERTY(QString value READ getValue CONSTANT NOTIFY valueChanged)
public:

    DeviceVariable(OICDeviceResource* res, OICDevice *dev);


signals:
    void valueChanged(QString path, QVariantMap val);


public slots:
    QString getResourceType(){return QString(m_resource->getResourceType().c_str());}
    QString getInterface(){return QString(m_resource->getInterface().c_str());}
    QString getHref(){return QString(m_resource->getHref().c_str());}
    QString getValue() {return m_value;}

    void post(QString value);
    void get();
    void observe();
    void unobserve();
private:
    void dump(cbor* res);
    QVariantMap toQMap(cbor* map);
    void convertToCborMap(QString str, cbor* map);
    OICDeviceResource* m_resource;
    OICDevice* m_device;
    QString m_value;
};

class Device : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ getName CONSTANT)
    Q_PROPERTY(QString id READ getID CONSTANT)
    Q_PROPERTY(QVariantList variables READ getVariables NOTIFY variablesChanged)

public:
    explicit Device(OICDevice* dev, QObject *parent);

    QString getID() {return m_device->getId().c_str();}
    QString getName() {return QString(m_device->getName().c_str());}

    QVariantList getVariables();

    QList<DeviceVariable*>* getVariablesList() {return &m_variables;}

    QString getAddress() { return QString(m_device->getAddress().c_str());}

signals:
    void variablesChanged(QString path, QVariantMap val);
    void setVariableValue(QString url, qint32 value);


private:
    OICDevice* m_device;
    QList<DeviceVariable*> m_variables;

};

#endif // DEVICE_H
