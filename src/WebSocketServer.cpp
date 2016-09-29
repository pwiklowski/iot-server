#include "WebSocketServer.h"
#include "QJsonDocument"
#include "QJsonObject"
#include "QJsonArray"

WebSocketServer::WebSocketServer(SmartHomeServer* server) : QObject(server)
{
    m_server = server;
    connect(m_server, SIGNAL(valueChanged(QString,QString,QVariantMap)), this, SLOT(onValueChanged(QString, QString,QVariantMap)));

    m_webSocketServer = new QWebSocketServer("wiklosoft_iot", QWebSocketServer::NonSecureMode, this);
    if (m_webSocketServer->listen(QHostAddress::Any, 7002)) {
        connect(m_webSocketServer, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
    }
}

void WebSocketServer::onNewConnection()
{
    QWebSocket *socket = m_webSocketServer->nextPendingConnection();

    qDebug() << "new connection" << socket->requestUrl();\

    m_socketList.append(socket);


    connect(socket, SIGNAL(textMessageReceived(QString)), this, SLOT(processTextMessage(QString)));
    connect(socket, SIGNAL(disconnected()), this,  SLOT(socketDisconnected()));

}


void WebSocketServer::processTextMessage(QString message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    QString url  = pClient->requestUrl().path();

    qDebug() << "processTextMessage" << pClient->requestUrl().path() << message;


    QRegExp deviceValue("/device/(.+)/value");



    if (deviceValue.indexIn(url) == 0){
        QJsonDocument json =  QJsonDocument::fromJson(message.toLatin1());
        QString id = deviceValue.cap(1);


        IotDevice* device = m_server->getDeviceById(id);

        QString resource = json.object().value("resource").toString();
        QJsonObject value = json.object().value("value").toObject();


        IotDeviceVariable* variable = device->getVariable(resource);

        variable->set(value.toVariantMap());

    }
}
void WebSocketServer::socketDisconnected()
{
    qDebug() << "socketDisconnected";
    QWebSocket* client = qobject_cast<QWebSocket *>(sender());
    m_socketList.removeAll(client);
    if (client) {
        client->deleteLater();
    }
}


void WebSocketServer::onValueChanged(QString id, QString resource, QVariantMap value){
    qDebug() << "WebSocket onValueChanged" << id << resource << value;


    QJsonObject obj;
    obj.insert("resource", resource);
    obj.insert("value", QJsonObject::fromVariantMap(value));


    foreach (QWebSocket* s, m_socketList) {

        s->sendTextMessage(QJsonDocument(obj).toJson());

    }



}
