#include "DjangoInterface.h"
#include "QJsonDocument"
#include "QJsonObject"
#include "QJsonArray"
#include "QDataStream"

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
    //qDebug() << "parseMessages" << type;
    switch (type) {
        case DJANGO_GET_DEVICE_LIST:
        {
            QList<Device*> devices = m_controller->getClientList();
            QJsonObject root;
            QString json;
            QJsonArray devs;

            for(int i=0; i<devices.length();i++)
            {
                Device* device = devices.at(i);

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
        case DJANGO_GET_VALUES_LIST:
        {
            QJsonDocument d = QJsonDocument::fromJson(payload);
            QJsonObject request = d.object();
            QString id = request["client_id"].toString();

            QString json;

            QVariantMap* storedVariables = m_controller->getVariablesStorage(id);



            QJsonArray vars;

            if (storedVariables)
            {
                foreach(QString key, storedVariables->keys())
                {
                    QVariantMap res = storedVariables->value(key).toMap();

                    QJsonObject v;
                    v["name"] = key;
                    v["values"]= QJsonObject::fromVariantMap(res);
                    vars.append(v);





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
    //QTcpSocket* client = (QTcpSocket*) sender();
    //delete client;
    //client =  0;
}
