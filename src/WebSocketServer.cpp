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
    QWebSocket *socket = qobject_cast<QWebSocket *>(sender());
    QString url  = socket->requestUrl().path();

    qDebug() << "processTextMessage" << message;


    QRegExp deviceValue("/device/(.+)/value");


    QJsonObject msg =  QJsonDocument::fromJson(message.toLatin1()).object();

    QJsonObject payload = msg.value("payload").toObject();
    int mid = msg.value("mid").toInt(-1);

    QString request = payload.value("request").toString();
    if (request == "GET_DEVICES"){
        QJsonObject response;
        response.insert("mid",mid);


        QList<IotDevice*> devices = m_server->getClientList();
        QJsonObject root;
        QString json;
        QJsonArray devs;

        for(int i=0; i<devices.length();i++)
        {
            IotDevice* device = devices.at(i);
            QJsonObject dev;
            dev["name"] = device->getName();
            dev["id"] = device->getID().remove("device:");

            QJsonArray vars;
            for(int i=0; i<device->getVariables()->size(); i++){
                IotDeviceVariable* var = device->getVariables()->at(i);
                vars.append(QJsonValue::fromVariant(var->getResource()));
            }
            dev["variables"] = vars;

            devs.append(dev);
        }
        root.insert("devices", devs);
        json = QJsonDocument(root).toJson(QJsonDocument::Compact);

        response.insert("payload", root);

        socket->sendTextMessage(QJsonDocument(response).toJson());
    }else if(request == "SET_VALUE"){
        QString id = payload.value("di").toString();
        QString resource = payload.value("resource").toString();
        QVariantMap value = payload.value("value").toObject().toVariantMap();

        IotDevice* device = m_server->getDeviceById(id);
        if (device){
            IotDeviceVariable* variable = device->getVariable(resource);
            variable->set(value);
        }

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
    obj.insert("method", "VALUE");

    QJsonObject payload;
    payload.insert("di", id);
    payload.insert("resource", resource);
    payload.insert("value", QJsonObject::fromVariantMap(value));

    obj.insert("payload", payload);

    foreach (QWebSocket* s, m_socketList) {
        //todo update only to instrested websockts, get url and check if id matches
        s->sendTextMessage(QJsonDocument(obj).toJson());

    }



}
