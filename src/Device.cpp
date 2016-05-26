#include "Device.h"
#include "QJsonDocument"
#include "QJsonObject"
#include "QJsonArray"
#include "QDebug"
#include "QDateTime"

Device::Device(OICDevice *dev, QObject* parent) :
    IotDevice(0)
{
    m_device = dev;
    m_type = IOT_DEVICE_TYPE_OCF;
    m_id =  m_device->getId().c_str();
    m_name = m_device->getName().c_str();

    for(size_t i=0; i<dev->getResources()->size(); i++)
    {
        DeviceVariable* v = new DeviceVariable(m_device->getResources()->at(i), dev);
        v->moveToThread(parent->thread());
        v->observe();

        connect(v, SIGNAL(valueChanged(QString,QString,QVariantMap)), this, SIGNAL(variablesChanged(QString, QString, QVariantMap)));
        m_variables.append(v);
    }
}


DeviceVariable::DeviceVariable(OICDeviceResource *res, OICDevice *dev):
    IotDeviceVariable(0)
{
    m_device = dev;
    m_resource = res;
    m_resourceURI = m_resource->getHref().c_str();
}

QVariantMap DeviceVariable::toQMap(cbor* map){
    QVariantMap m;
    Map<cbor, cbor>* cmap = map->toMap();

    for(cbor key: *cmap){
        cbor value = cmap->get(key);

        if (value.is_int()){
            m.insert(key.toString().c_str(), value.toInt());
        } else if (value.is_string()){
            m.insert(key.toString().c_str(), value.toString().c_str());
        }

    }
    return m;
}
void DeviceVariable::set(QVariantMap value){
    qDebug() << "post" << m_resource->getHref().c_str() << value;

    foreach (QString k, value.keys()){
        m_value.toMap()->insert(k.toLatin1().data(), value.value(k).toInt());
    }

    //emit valueChanged();
    m_resource->post(&m_value, [&] (COAPPacket* response){
        if (response == 0){
            qDebug() << "post timeout";
            return;
        }
        qDebug() << "value set";
    });
}

void DeviceVariable::observe(){
    m_resource->observe([&] (COAPPacket* response){
        if (response == 0){
            qDebug() << "observe timeout";
            return;
        }
        cbor::parse(&m_value, response->getPayload());
        qDebug() << "Value updated" << m_resource->getHref().c_str();
        emit valueChanged(m_device->getId().c_str(), m_resource->getHref().c_str(), toQMap(&m_value));
    });
}
void DeviceVariable::unobserve(){
    m_resource->unobserve([&] (COAPPacket* response){

    });
}
