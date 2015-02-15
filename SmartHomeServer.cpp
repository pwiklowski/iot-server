#include "SmartHomeServer.h"

SmartHomeServer::SmartHomeServer(QObject *parent) :
    QObject(parent)
{
    m_server.listen(QHostAddress::Any, 9999);

    connect(&m_server, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));
}


void SmartHomeServer::handleNewConnection()
{
    qDebug() << "handleNewConnection()";

    QTcpSocket* socket = m_server.nextPendingConnection();

    SmartHomeClient* client = new SmartHomeClient(socket, this);
    connect(client, SIGNAL(disconnected()), this, SLOT(removeClient()));
    connect(this, SIGNAL(clientListChanged()), client, SLOT(deviceListChanged()));

    m_clientList.append(client);

}


QList<SmartHomeClient*> SmartHomeServer::getClientList()
{
    return m_clientList;
}


void SmartHomeServer::removeClient()
{
    qDebug() << "removeClient";
    SmartHomeClient* client = (SmartHomeClient*) sender();
    disconnect(this, SIGNAL(clientListChanged()), client, SLOT(deviceListChanged()));

    delete client;
    m_clientList.removeOne(client);
    emit clientListChanged();
}
