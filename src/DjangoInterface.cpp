#include "DjangoInterface.h"
#include "QJsonDocument"
#include "QJsonObject"
#include "QJsonArray"

DjangoInterface::DjangoInterface(SmartHomeServer* controller, QObject *parent) :
    QObject(parent),
    m_controller(controller)
{

    m_server.listen(QHostAddress::Any, 5000);

    connect(&m_server, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));
}


void DjangoInterface::handleNewConnection()
{
    //qDebug() << "DjangoInterface::handleNewConnection()";

    QTcpSocket* socket = m_server.nextPendingConnection();
    connect(socket, SIGNAL(disconnected()), this, SLOT(removeClient()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(readData()));
}

void DjangoInterface::readData()
{
    QTcpSocket* client = (QTcpSocket*) sender();
    QByteArray buffer;

    buffer.append(client->readAll());

   quint8 type;
    quint16 size;

    QDataStream stream(&buffer, QIODevice::ReadOnly);

    stream >> type;
    stream >> size;

    while (buffer.size() < 3+ size)
    {
        client->waitForReadyRead();
        if (client == 0)
            return;
        buffer.append(client->readAll());
    }
    QByteArray payload = buffer.mid(3, size);
    buffer.remove(0, 3+size);
    parseMessages(type, payload, client);

}
void DjangoInterface::parseMessages(quint8 type, QByteArray payload, QTcpSocket* socket)
{
    switch (type) {
        case DJANGO_GET_DEVICE_LIST:
        {
            QList<SmartHomeClient*> devices = m_controller->getClientList();
            QJsonObject root;
            QString json;
            QJsonArray devs;

            for(int i=0; i<devices.length();i++)
            {
                SmartHomeClient* device = devices.at(i);

                if (device->getID().startsWith("controller"))
                    continue;
                QJsonObject dev;
                dev["name"] = device->getName();
                dev["id"] = device->getID().remove("device:");

                devs.append(dev);
            }
            root.insert("devices", devs);
            json = QJsonDocument(root).toJson();

            QByteArray data;
            QDataStream stream(&data, QIODevice::ReadWrite);

            stream << static_cast<quint8>(DJANGO_DEVICE_LIST);
            stream << static_cast<quint16>(json.size());

            for(int i=0; i< json.size();i++)
                stream << static_cast<quint8>(json.at(i).toLatin1());

            socket->write(data);
            socket->flush();
            break;
        }
        case DJANGO_GET_BUTTON_LIST:
        {
            QMap<QString, QVariantMap> lastEvents = m_controller->getLastEventMap();
            QJsonObject root;
            QString json;
            QJsonArray buttons;

            QStringList sources = lastEvents.keys();

            for(int i=0;i<sources.size();i++)
            {
                QString source = sources.at(i);
                QVariantMap parameters = lastEvents.value(source);


                QJsonObject params;
                params["source"] = source;

                params["last"] = QString(QJsonDocument::fromVariant(parameters).toJson());

                buttons.append(params);
            }
            root.insert("buttons", buttons);
            json = QJsonDocument(root).toJson();

            QByteArray data;
            QDataStream stream(&data, QIODevice::ReadWrite);

            stream << static_cast<quint8>(DJANGO_BUTTON_LIST);
            stream << static_cast<quint16>(json.size());

            for(int i=0; i< json.size();i++)
                stream << static_cast<quint8>(json.at(i).toLatin1());

            socket->write(data);
            socket->flush();
            break;
        }
        case DJANGO_GET_DEVICE_DESCRIPTION:
        {
            QJsonDocument d = QJsonDocument::fromJson(payload);
            QJsonObject request = d.object();
            QString id = request["client_id"].toString();

            SmartHomeClient* client = m_controller->getClient("device:"+id);


            QString json;

            if (client !=0)
            {
                json = client->getDescription();
            }

            QByteArray data;
            QDataStream stream(&data, QIODevice::ReadWrite);

            stream << static_cast<quint8>(DJANGO_DEVICE_DESCRIPTION);
            stream << static_cast<quint16>(json.size());

            for(int i=0; i< json.size();i++)
                stream << static_cast<quint8>(json.at(i).toLatin1());

            socket->write(data);
            socket->flush();
            break;
        }



        case DJANGO_GET_SENSORS_LIST:
        {
            QMap<QString, QVariantMap> lastEvents = m_controller->getSensorsMap();
            QJsonObject root;
            QString json;
            QJsonArray buttons;

            QStringList sources = lastEvents.keys();

            foreach(QString source, sources)
            {
                QVariantMap parameters = lastEvents.value(source);


                QJsonObject params;
                params["source"] = source;
                params["last"] = QString(QJsonDocument::fromVariant(parameters).toJson());

                buttons.append(params);
            }
            root.insert("sensors", buttons);
            json = QJsonDocument(root).toJson();

            QByteArray data;
            QDataStream stream(&data, QIODevice::ReadWrite);

            stream << static_cast<quint8>(DJANGO_SENSORS_LIST);
            stream << static_cast<quint16>(json.size());

            for(int i=0; i< json.size();i++)
                stream << static_cast<quint8>(json.at(i).toLatin1());

            socket->write(data);
            socket->flush();
            break;
        }
//        case DeviceController::CHANGE_VALUE:
//        {
//            QJsonDocument d = QJsonDocument::fromJson(payload);
//            QJsonObject root = d.object();
//            QString id = root["id"].toString();
//            QString variable = root["variable"].toString();
//            quint32 value = root["value"].toInt();

//            m_controller->sendChangeVariable(id, variable, value);

//            socket->write("OK");
//            socket->flush();
//            break;
//        }
        case DJANGO_GET_VALUES_LIST:
        {
            QJsonDocument d = QJsonDocument::fromJson(payload);
            QJsonObject request = d.object();
            QString id = request["client_id"].toString();

            QString json;

            QVariantMap* storedVariables = m_controller->getVariablesStorage("device:"+id);



            QJsonObject vars;

            if (storedVariables)
            {
                foreach(QString key, storedVariables->keys())
                {
                    vars[key]= storedVariables->value(key).toInt();
                }
            }


            json = QJsonDocument(vars).toJson();
            QByteArray data;
            QDataStream stream(&data, QIODevice::ReadWrite);

            stream << static_cast<quint8>(DJANGO_VALUES_LIST);
            stream << static_cast<quint16>(json.size());

            for(int i=0; i< json.size();i++)
                stream << static_cast<quint8>(json.at(i).toLatin1());

            socket->write(data);
            socket->flush();
            break;
            break;
        }
    }
}

void DjangoInterface::removeClient()
{
    //qDebug() << "DjangoInterface::removeClient";
    QTcpSocket* client = (QTcpSocket*) sender();
    delete client;
    client =  0;
}
