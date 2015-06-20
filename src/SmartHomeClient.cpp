#include "SmartHomeClient.h"
#include "SmartHomeServer.h"
#include "QHostAddress"
#include "QJsonDocument"
#include "QJsonObject"
#include "QJsonArray"


SmartHomeClient::SmartHomeClient(QTcpSocket* socket, SmartHomeServer* parent) :
    QObject(parent),
    m_socket(socket)
{
    m_server = parent;
    qDebug() << "New client " << socket->peerAddress() << socket->peerPort();

    connect(m_socket, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(m_socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
    getDeviceDescription();


   m_ping_timer.setSingleShot(false);
   m_ping_timer.setInterval(3000);
   connect(&m_ping_timer, SIGNAL(timeout()), this, SLOT(sendPing()));
   m_ping_timer.start();

   m_remove_timer.setSingleShot(true);
   m_remove_timer.setInterval(2000);
   connect(&m_remove_timer, SIGNAL(timeout()), this, SLOT(pingTimeout()));



}
void SmartHomeClient::pingTimeout()
{
    qDebug() << "pingTimeout" << m_name;
    m_socket->close();
}

void SmartHomeClient::readData()
{
    m_buffer.append(m_socket->readAll());

    if (m_buffer.size() < 7) return;
    quint32 header;
    quint8 type;
    quint16 size;

    QDataStream stream(m_buffer);

    stream >> header;
    if (header != SMART_HOME_HEADER)
    {
        qWarning() << m_name << "Invalid header";
        m_buffer.remove(0,1);
        readData();
    }

    stream >> type;
    stream >> size;

    if (m_buffer.size() < 7 + size)
    {
        qWarning() << "Not complete message" << m_buffer.size() << (7+size);
        return;
    }
    QByteArray payload = m_buffer.mid(7, size);

    m_buffer.remove(0, 7+size);
    parseMessages(type, payload);

    if (m_buffer.size() >0) readData();
}
void SmartHomeClient::parseMessages(quint8 type, QByteArray payload)
{
    //qDebug() << "parseMessages" << type;
    switch (type) {
    case DEVICE_DESCRIPTION:
    {
        m_description = QString(payload);


        QJsonDocument d = QJsonDocument::fromJson(payload);
        QJsonObject root = d.object();
        m_id = root["id"].toString();
        m_name = root["name"].toString();

        QVariantMap* variablesMap =  new QVariantMap();

        QJsonArray variables = root["variables"].toArray();

        for(int i=0;i<variables.size();i++)
        {
            QJsonObject variable = variables.at(i).toObject();
            variablesMap->insert(variable["resource"].toString(), variable["value"].toInt());
        }
        m_server->getVariablesStorage()->insert(m_id,variablesMap);

        qDebug() << "DEVICE_DESCRIPTION" << m_id << m_name;

        if (!m_id.contains("controller:"))
            deviceListChanged();
        break;
    }
    case GET_DEVICE_LIST:
    {
        sendDeviceList();
        break;
    }
    case CHANGE_VALUE:
    {
        QJsonDocument d = QJsonDocument::fromJson(payload);
        QJsonObject root = d.object();
        QString id = root["id"].toString();

        QList<SmartHomeClient*> clients = m_server->getClientList();

        for(int i=0; i<clients.size();i++)
        {
            if (clients.at(i)->getID() == id)
            {
                clients.at(i)->sendChangeValue(payload);
                break;
            }
        }
        break;
    }
    case VALUE_CHANGED:
    {
        QList<SmartHomeClient*> clients = m_server->getClientList();

        QJsonDocument d = QJsonDocument::fromJson(payload);
        QJsonObject root = d.object();
        QString id = root["id"].toString();


        QVariantMap* storedVariables = m_server->getVariablesStorage(id);

        storedVariables->insert(root["resource"].toString(), root["value"].toInt());




        for(int i=0; i<clients.size();i++)
        {
            if (clients.at(i)->getID().contains("controller:"))
            {
                clients.at(i)->sendVariableChanged(payload);
            }
        }
        break;
    }
    case GET_VALUES_LIST:
    {
        QJsonDocument d = QJsonDocument::fromJson(payload);
        QJsonObject root = d.object();
        QString id = root["client_id"].toString();

        QList<SmartHomeClient*> clients = m_server->getClientList();

        for(int i=0; i<clients.size();i++)
        {
            if (clients.at(i)->getID() == id)
            {
                clients.at(i)->sendGetValuesList(payload);
                break;
            }
        }


        break;
    }
    case VALUES_LIST:
    {
        QJsonDocument d = QJsonDocument::fromJson(payload);
        QJsonObject root = d.object();
        QString client_id = root["client_id"].toString();

        QString id = root["client_id"].toString();

        QVariantMap* storedVariables = m_server->getVariablesStorage(id);

        QJsonArray variables = root["variables"].toArray();

        for(int i=0;i<variables.size();i++)
        {
            QJsonObject variable = variables.at(i).toObject();
            storedVariables->insert(variable["resource"].toString(), variable["value"].toInt());
        }




        QList<SmartHomeClient*> clients = m_server->getClientList();

        for(int i=0; i<clients.size();i++)
        {
            if (clients.at(i)->getID() == client_id)
            {
                clients.at(i)->sendValuesList(payload);
                break;
            }
        }

        break;
    }
    case PING_RESPONSE:
    {
        qDebug() << m_id << "PING_RESPONSE" << 2000-m_remove_timer.remainingTime();
        m_remove_timer.stop();
    }
    default:
        break;
    }
}
void SmartHomeClient::sendValuesList(QByteArray payload)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::ReadWrite);

    stream << static_cast<quint32>(SMART_HOME_HEADER);
    stream << static_cast<quint8>(VALUES_LIST);
    stream << static_cast<quint16>(payload.size());

    for(int i=0; i< payload.size();i++)
        stream << static_cast<quint8>(payload.at(i));

    m_socket->write(data);
    m_socket->flush();

}
void SmartHomeClient::sendGetValuesList(QByteArray payload)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::ReadWrite);

    stream << static_cast<quint32>(SMART_HOME_HEADER);
    stream << static_cast<quint8>(GET_VALUES_LIST);
    stream << static_cast<quint16>(payload.size());

    for(int i=0; i< payload.size();i++)
        stream << static_cast<quint8>(payload.at(i));

    m_socket->write(data);
    m_socket->flush();

}
void SmartHomeClient::sendPing()
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::ReadWrite);

    stream << static_cast<quint32>(SMART_HOME_HEADER);
    stream << static_cast<quint8>(PING);
    stream << static_cast<quint16>(0);

    m_socket->write(data);
    m_socket->flush();

    m_remove_timer.start();
}
void SmartHomeClient::sendChangeValue(QByteArray payload)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::ReadWrite);

    stream << static_cast<quint32>(SMART_HOME_HEADER);
    stream << static_cast<quint8>(CHANGE_VALUE);
    stream << static_cast<quint16>(payload.size());

    for(int i=0; i< payload.size();i++)
        stream << static_cast<quint8>(payload.at(i));

    m_socket->write(data);
    m_socket->flush();

}
void SmartHomeClient::sendChangeValue(QString resource, qint32 value)
{
    //qDebug() << "SmartHomeClient::SmartHomeClient" << m_id << resource << value;

    QString json;
    QJsonObject root;

    root["id"] = m_id;
    root["resource"] = resource;
    root["value"] = value;

    json = QJsonDocument(root).toJson();

    QByteArray data;
    QDataStream stream(&data, QIODevice::ReadWrite);

    stream << static_cast<quint32>(SMART_HOME_HEADER);
    stream << static_cast<quint8>(CHANGE_VALUE);
    stream << static_cast<quint16>(json.size());

    for(int i=0; i< json.size();i++)
        stream << static_cast<quint8>(json.at(i).toLatin1());

    m_socket->write(data);
    m_socket->flush();


}
void SmartHomeClient::sendVariableChanged(QByteArray payload)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::ReadWrite);

    stream << static_cast<quint32>(SMART_HOME_HEADER);
    stream << static_cast<quint8>(VALUE_CHANGED);
    stream << static_cast<quint16>(payload.size());

    for(int i=0; i< payload.size();i++)
        stream << static_cast<quint8>(payload.at(i));

    m_socket->write(data);
    m_socket->flush();
}
void SmartHomeClient::getDeviceDescription()
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::ReadWrite);

    stream << static_cast<quint32>(SMART_HOME_HEADER);
    stream << static_cast<quint8>(GET_DEVICE_DESCRIPTION);
    stream << static_cast<quint16>(0);

    m_socket->write(data);
    m_socket->flush();
}

QString SmartHomeClient::generateDeviceList()
{
    QString deviceList;

    QJsonDocument d;
    QJsonObject root = d.object();

    QJsonArray devices;

    QList<SmartHomeClient*> clients = m_server->getClientList();

    for(int i=0; i<clients.size();i++)
    {
        if (clients.at(i)->getID() == m_id) continue;
        if (clients.at(i)->getID().contains("controller:")) continue;

        QJsonObject device;
        device["name"] = clients.at(i)->getName();
        device["id"] = clients.at(i)->getID();
        device["description"] = clients.at(i)->getDescription();
        devices.append(device);
    }

    root["devices"] = devices;

    deviceList.append(QJsonDocument(root).toJson());

    return deviceList;
}
void SmartHomeClient::sendDeviceList()
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::ReadWrite);

    stream << static_cast<quint32>(SMART_HOME_HEADER);
    stream << static_cast<quint8>(DEVICE_LIST);

    QString deviceList = generateDeviceList();

    stream << static_cast<quint16>(deviceList.count());
    for(int i=0; i< deviceList.size();i++)
        stream << static_cast<quint8>(deviceList.at(i).toLatin1());

    m_socket->write(data);
    m_socket->flush();
}
void SmartHomeClient::sendDeviceListChanged()
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::ReadWrite);

    stream << static_cast<quint32>(SMART_HOME_HEADER);
    stream << static_cast<quint8>(DEVICE_LIST_CHANGED);

    QString deviceList = generateDeviceList();

    stream << static_cast<quint16>(deviceList.count());
    for(int i=0; i< deviceList.size();i++)
        stream << static_cast<quint8>(deviceList.at(i).toLatin1());

    m_socket->write(data);
    m_socket->flush();
}
void SmartHomeClient::deviceListChanged()
{
    QList<SmartHomeClient*> clients = m_server->getClientList();
    for(int i=0; i<clients.size();i++)
    {
        if (clients.at(i)->getID().contains("controller:"))
        {
            clients.at(i)->sendDeviceListChanged();
        }
    }
}
void SmartHomeClient::close(){
    if (m_socket && m_socket->isOpen())
        m_socket->close();

}
