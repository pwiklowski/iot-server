#include "Device.h"
#include "QJsonDocument"
#include "QJsonObject"
#include "QJsonArray"
#include "QDebug"
#include "QDateTime"

Device::Device(OICDevice *dev, QObject* parent) :
    QObject(0)
{
    m_device = dev;
    for(size_t i=0; i<dev->getResources()->size(); i++)
    {
        DeviceVariable* v = new DeviceVariable(m_device->getResources()->at(i), dev);
        v->moveToThread(parent->thread());
        v->observe();

        connect(v, SIGNAL(valueChanged(QString,QString,QVariantMap)), this, SIGNAL(variablesChanged(QString, QString, QVariantMap)));
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

DeviceVariable* Device::getVariable(QString resource){
    for(int i=0; i<m_variables.length();i++){
        if(m_variables.at(i)->getHref() == resource) return m_variables.at(i);
    }

    return 0;
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
            map->append(key.toLatin1().data(), integer);
            qDebug() << key << integer;
        } else{
            qDebug() << key << v;
            map->append(key.toLatin1().data(), v.toLatin1().data());
        }

    }
}
void DeviceVariable::postJson(QString value){
    qDebug() << "post" << m_resource->getHref().c_str() << value;

    cbor m(CBOR_TYPE_MAP);

    convertToCborMap(value, &m);
    m_value = m;

    m_resource->post(&m_value, [&] (COAPPacket* response){
        if (response == 0){
            qDebug() << "setting value failed. timeout";
        }else{
            qDebug() << "value set";
        }
    });

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

void DeviceVariable::post(QVariantMap value){
    qDebug() << "postJson" << m_resource->getHref().c_str() << value;


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
void DeviceVariable::get(){
    qDebug() << QDateTime::currentMSecsSinceEpoch() << "DeviceVariable::get" << getResourceType() << getHref();

    m_resource->get([&] (COAPPacket* response){
        if (response == 0){
            qDebug() << "get timeout";
            return;
        }

        cbor::parse(&m_value, response->getPayload());
        //emit valueChanged();
        qDebug() << QDateTime::currentMSecsSinceEpoch() << "DeviceVariable::get" << getResourceType() << getHref();
    });
}

QString DeviceVariable::dump(cbor* response){
    QString res;
    Map<cbor, cbor>* m = response->toMap();

    if (m!=0){
        for (cbor key: *m){
            cbor value = m->get(key);
            res += new QString(key.toString().c_str());
            res += ": ";

            if (value.is_int()){
                res += QString::number(value.toInt());
            }else if(value.is_string()){
                res += new QString(value.toString().c_str());
            }
            res +="\n";
        }
    }
    return res;
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


QString DeviceVariable::getValue(){
    return dump(&m_value);
}
