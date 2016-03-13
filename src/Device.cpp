#include "Device.h"
#include "QJsonDocument"
#include "QJsonObject"
#include "QJsonArray"
#include "QDebug"
#include "QDateTime"
#include "cbor.h"

Device::Device(OICDevice *dev, QObject* parent) :
    QObject(0)
{
    m_device = dev;
    for(size_t i=0; i<dev->getResources()->size(); i++)
    {
        DeviceVariable* v = new DeviceVariable(m_device->getResources()->at(i), dev);
        v->moveToThread(parent->thread());
        v->observe();

        connect(v, SIGNAL(valueChanged(QString,QVariantMap)), this, SIGNAL(variablesChanged(QString, QVariantMap)));
        m_variables.append(v);
    }
}

QVariantList Device::getVariables()
{
    QVariantList list;
    for(int i=0; i<m_variables.length();i++)
        list.append(QVariant::fromValue(m_variables.at(i)));

    return list;
}

DeviceVariable::DeviceVariable(OICDeviceResource *res, OICDevice *dev):
    QObject(0)
{
    m_device = dev;
    m_resource = res;
}

void DeviceVariable::convertToCborMap(QString str, cbor* map){
    if (str.isEmpty()) return;
    QStringList t = str.split("\n", QString::SkipEmptyParts);
    for(QString s: t){
        QStringList val = s.split(":", QString::SkipEmptyParts);
        QString v = val.at(1).trimmed();
        QString key = val.at(0).trimmed();
        bool ok;
        int integer = v.toInt(&ok);

        if (ok){
            map->append(new cbor(key.toLatin1().data()), new cbor(integer));
            qDebug() << key << integer;
        } else{
            qDebug() << key << v;
            map->append(new cbor(key.toLatin1().data()), new cbor(v.toLatin1().data()));
        }

    }
}


QVariantMap DeviceVariable::toQMap(cbor* map){
    QVariantMap m;
    Map<cbor*, cbor*>* cmap = map->toMap();

    if (cmap != 0){
        for(cbor* key: *cmap){
            cbor* value = cmap->get(key);

            if (value->is_int()){
                m.insert(key->toString().c_str(), value->toInt());
            } else if (value->is_string()){
                m.insert(key->toString().c_str(), value->toString().c_str());
            }

        }

    }
    return m;
}

void DeviceVariable::post(QString value){
    qDebug() << "post" << m_resource->getHref().c_str() << m_value;

    cbor* payload = cbor::map();

    convertToCborMap(value, payload);

    //payload->append(new cbor("dimmingSetting"), new cbor(50));

    m_resource->post(payload, [&] (COAPPacket* response){
        qDebug() << "value set";
    });

}

void DeviceVariable::get(){
    qDebug() << QDateTime::currentMSecsSinceEpoch() << "DeviceVariable::get" << getResourceType() << getHref();

    m_resource->get([&] (COAPPacket* response){
        cbor* res = cbor::parse(response->getPayload());
        dump(res);

        emit valueChanged(m_resource->getHref().c_str(), toQMap(res));
        qDebug() << QDateTime::currentMSecsSinceEpoch() << "DeviceVariable::get" << getResourceType() << getHref();
    });
}

void DeviceVariable::dump(cbor* res){
    m_value ="";
    Map<cbor*, cbor*>* m = res->toMap();

    if (m!=0){
        for (cbor* key: *m){
            cbor* value = m->get(key);
            m_value += new QString(key->toString().c_str());
            m_value += ": ";

            if (value->is_int()){
                m_value += QString::number(value->toInt());
            }else if(value->is_string()){
                m_value += new QString(value->toString().c_str());
            }
            m_value +="\n";
        }
    }
}


void DeviceVariable::observe(){
    m_resource->observe([&] (COAPPacket* response){
        cbor* res = cbor::parse(response->getPayload());
        dump(res);
        emit valueChanged(m_resource->getHref().c_str(), toQMap(res));
    });
}
void DeviceVariable::unobserve(){
    m_resource->unobserve([&] (COAPPacket* response){

    });
}
