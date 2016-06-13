#ifndef IOTDEVICE_H
#define IOTDEVICE_H

#include <QObject>


typedef enum{
    IOT_DEVICE_TYPE_OCF,
    IOT_DEVICE_TYPE_BLE_CHINA_BUTTON

}IotDeviceType;

class IotDeviceVariable : public QObject{
    Q_OBJECT
public:
    IotDeviceVariable(QObject* parent);


    virtual void set(QVariantMap value) =0;

    QString getResource() {return m_resourceURI;}

protected:
    QString m_resourceURI;

};

class IotDevice : public QObject
{
    Q_OBJECT
public:
    explicit IotDevice(QObject *parent = 0);
    QString getID() { return m_id;}
    QString getName() { return m_name;}


    IotDeviceVariable* getVariable(QString resource);
    QList<IotDeviceVariable*>* getVariables() {return &m_variables;}

signals:

public slots:


protected:
    IotDeviceType m_type;
    QList<IotDeviceVariable*> m_variables;
    QString m_id;
    QString m_name;

};

#endif // IOTDEVICE_H
