#include "WebSocketServer.h"
#include "QJsonDocument"
#include "QJsonObject"
#include "QJsonValue"
#include "QJsonArray"

WebSocketServer::WebSocketServer(SmartHomeServer* server) : QObject(server)
{
    m_server = server;
    connect(m_server, SIGNAL(valueChanged(QString,QString,QVariantMap)), this, SLOT(onValueChanged(QString, QString,QVariantMap)));
    connect(m_server, SIGNAL(devicesChanged()), this, SLOT(onDeviceListUpdate()));
    connect(m_server, SIGNAL(newLogMessage(QString,QString)), this, SLOT(onLogMessage(QString, QString)));

    m_webSocketServer = new QWebSocketServer("wiklosoft_iot", QWebSocketServer::NonSecureMode, this);
    if (m_webSocketServer->listen(QHostAddress::Any, 7002)) {
        connect(m_webSocketServer, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
    }
}

void WebSocketServer::onNewConnection()
{
    QWebSocket *socket = m_webSocketServer->nextPendingConnection();

    qDebug() << "new connection" << socket->requestUrl();\

    m_socketList.append(new WebSocketConnection(socket));

    connect(socket, SIGNAL(textMessageReceived(QString)), this, SLOT(processTextMessage(QString)));
    connect(socket, SIGNAL(disconnected()), this,  SLOT(socketDisconnected()));
}
WebSocketConnection* WebSocketServer::getSocketConnection(QWebSocket* socket){
    foreach (WebSocketConnection* s, m_socketList) {
        if (s->getSocket() == socket){
            return  s;
        }
    }
    return 0;
}


void WebSocketServer::processTextMessage(QString message)
{
    QWebSocket *websocket = qobject_cast<QWebSocket *>(sender());
    WebSocketConnection* connection = getSocketConnection(websocket);

    QString url  = connection->getSocket()->requestUrl().path();

    qDebug() << "processTextMessage" << message;

    QRegExp deviceValue("/device/(.+)/value");

    QJsonObject msg =  QJsonDocument::fromJson(message.toLatin1()).object();

    QJsonObject payload = msg.value("payload").toObject();
    int mid = msg.value("mid").toInt(-1);

    QString request = payload.value("request").toString();


    qDebug() << request;
    if (request == "RequestAuthorize"){
        QString token = payload.value("token").toString();
        bool res = m_server->hasAccess(token);

        qDebug() << "isAuthorized" << res;
        connection->setAuthorized(res);

        QJsonObject response;
        response.insert("mid",mid);
        response.insert("payload",res);

        connection->getSocket()->sendTextMessage(QJsonDocument(response).toJson());
    }

    if (!connection->isAuthorized()) return;


    if (request == "RequestGetDevices"){
        QJsonObject response;
        response.insert("mid",mid);
        response.insert("event", "EventDeviceListUpdate");


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

        connection->getSocket()->sendTextMessage(QJsonDocument(response).toJson());
    }else if(request == "RequestSetValue"){
        QString id = payload.value("di").toString();
        QString resource = payload.value("resource").toString();
        QVariantMap value = payload.value("value").toObject().toVariantMap();

        IotDevice* device = m_server->getDeviceById(id);
        if (device){
            IotDeviceVariable* variable = device->getVariable(resource);
            variable->set(value);
        }
    }else if(request == "RequestSubscribeDevice"){
        QString uuid = payload.value("uuid").toString();
        connection->getDeviceSubscription()->append(uuid);
        qDebug() << "RequestSubscribeDevice" << uuid;

    }else if(request == "RequestUnsubscribeDevice"){
        QString uuid = payload.value("uuid").toString();
        connection->getDeviceSubscription()->removeAll(uuid);
        qDebug() << "RequestUnsubscribeDevice" << uuid;

    }else if(request == "RequestSubscribeScript"){
        QString uuid = payload.value("uuid").toString();
        connection->getScriptSubscription()->append(uuid);
        qDebug() << "RequestSubscribeScript" << uuid;

    }else if(request == "RequestUnsubscribeScript"){
        QString uuid = payload.value("uuid").toString();
        connection->getScriptSubscription()->removeAll(uuid);
        qDebug() << "RequestUnsubscribeScript" << uuid;

    }else if(request == "RequestRunScript"){
        qDebug() << "RequestRunScript";

        QString id = payload.value("uuid").toString();
        QJsonObject obj = payload.value("object").toObject();
        m_server->runScriptId(id, obj.toVariantMap());
    }else if(request == "RequestGetDeviceResources"){
        QJsonObject response;
        response.insert("mid",mid);

        QString id = payload.value("uuid").toString();

        IotDevice* dev = m_server->getDeviceById(id);

        QVariantMap* storedVariables = m_server->getVariablesStorage(id);

        QJsonArray vars;
        if (dev)
        {
            for(int i=0; i<dev->getVariables()->size(); i++)
            {
                IotDeviceVariable* var = dev->getVariables()->at(i);
                QVariantMap res = storedVariables->value(var->getResource()).toMap();

                QJsonObject v;
                v["name"] = var->getResource();
                v["values"]= QJsonObject::fromVariantMap(res);
                vars.append(v);
            }
        }

        response.insert("payload", vars);

        qDebug() << QJsonDocument(response).toJson();

        connection->getSocket()->sendTextMessage(QJsonDocument(response).toJson());
    }


}
void WebSocketServer::socketDisconnected()
{
    qDebug() << "socketDisconnected";
    QWebSocket* client = qobject_cast<QWebSocket *>(sender());

    for(WebSocketConnection* connection: m_socketList){
        if (connection->getSocket() == client){
            m_socketList.removeAll(connection);
        }
    }
}

void WebSocketServer::onDeviceListUpdate(){
    QJsonObject event;
    event.insert("event", "EventDeviceListUpdate");

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

    event.insert("payload", root);

    foreach (WebSocketConnection* s, m_socketList) {
        s->getSocket()->sendTextMessage(QJsonDocument(event).toJson());

    }
}

void WebSocketServer::onValueChanged(QString id, QString resource, QVariantMap value){
    qDebug() << "WebSocket onValueChanged" << id << resource << value;

    QJsonObject obj;
    obj.insert("event", "EventValueUpdate");

    QJsonObject payload;
    payload.insert("di", id);
    payload.insert("resource", resource);
    payload.insert("value", QJsonObject::fromVariantMap(value));

    obj.insert("payload", payload);

    foreach (WebSocketConnection* s, m_socketList) {
        if (s->getDeviceSubscription()->contains(id)){
            s->getSocket()->sendTextMessage(QJsonDocument(obj).toJson());
            s->getSocket()->flush();
        }
    }
}


void WebSocketServer::onLogMessage(QString uuid, QString message){
    qDebug() << "onLogMessage" << uuid << message;
    QJsonObject obj;
    obj.insert("event", "EventLog");

    QJsonObject payload;
    payload.insert("uuid",uuid);
    obj.insert("payload", message);
    foreach (WebSocketConnection* s, m_socketList) {
        if (s->getScriptSubscription()->contains(uuid)){
            s->getSocket()->sendTextMessage(QJsonDocument(obj).toJson());
            s->getSocket()->flush();
        }
    }
}

