#include "WebSocketServer.h"
#include "SmartHomeServer.h"
#include "QJsonDocument"
#include "QJsonObject"
#include "QJsonValue"
#include "QJsonArray"
#include "QNetworkAccessManager"
#include "QNetworkRequest"
#include "QNetworkReply"
#include "QEventLoop"

#define IOT_CLOUD_URL "wss://iot.wiklosoft.com/connect"


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


    m_iotCloudConnection = new WebSocketConnection(new QWebSocket());
    m_iotCloudConnection->getSocket()->open(QUrl(IOT_CLOUD_URL));

    connect(m_iotCloudConnection->getSocket(), &QWebSocket::connected, [=](){
        QJsonObject config = m_server->readSettings();
        QString clientUuid = config.value("uuid").toString();
        QString name = config.value("name").toString();

        QString token = config.value("iot.token").toString();
        QString refresh_token =  config.value("iot.token.refresh_token").toString();
        long tokenEpire = config.value("iot.token.exp").toVariant().toLongLong();


        if (tokenEpire < QDateTime::currentMSecsSinceEpoch()){
            qDebug() << "refreshToken";
            refreshToken();
            config = m_server->readSettings();
            token = config.value("iot.token").toString();

        }
        QJsonObject authMessage;
        authMessage.insert("name", "RequestAuthorize");

        QJsonObject payload;
        payload.insert("token", token);
        payload.insert("uuid", clientUuid);
        payload.insert("name",name);
        authMessage.insert("payload", payload);


        m_iotCloudConnection->getSocket()->sendTextMessage(QJsonDocument(authMessage).toJson());
        m_iotCloudConnection->setAuthorized(true);
        m_socketList.append(m_iotCloudConnection);
    });
    connect(m_iotCloudConnection->getSocket(), &QWebSocket::disconnected, [=](){
        qDebug() << "ws disconnected" << m_iotCloudConnection->getSocket()->closeCode() << m_iotCloudConnection->getSocket()->closeReason();
        m_socketList.removeOne(m_iotCloudConnection);

        QTimer::singleShot(500, [=]{
            m_iotCloudConnection->getSocket()->open(QUrl(IOT_CLOUD_URL));
        });

    });

    connect(m_iotCloudConnection->getSocket(), &QWebSocket::textMessageReceived, [=](QString textMessage){
        processMessage(textMessage, m_iotCloudConnection);
    });



}
void WebSocketServer::refreshToken(){
    QJsonObject settings = m_server->readSettings();

    QString client = settings.value("auth.client").toString();
    QString secret = settings.value("auth.secret").toString();
    QString refresh_token = settings.value("iot.token.refresh_token").toString();


    QUrl url("https://auth.wiklosoft.com/v1/oauth/tokens");

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QString concatenated =  client+ ":" +secret;
    QByteArray data = concatenated.toLocal8Bit().toBase64();
    QString headerData = "Basic " + data;
    req.setRawHeader("Authorization", headerData.toLocal8Bit());

    QNetworkAccessManager network;

    QNetworkReply *reply = network.post(req,QString("grant_type=refresh_token&refresh_token="+refresh_token).toLatin1());

    QEventLoop loop;
    QObject::connect(reply, SIGNAL(readyRead()), &loop, SLOT(quit()));
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));

    loop.exec();

    QString response = reply->readAll();

    QJsonObject doc = QJsonDocument::fromJson(response.toLatin1()).object();

    QString newToken = doc.value("access_token").toString();
    QString newRefreshToken = doc.value("refresh_token").toString();
    long tokenEpire = doc.value("expires_in").toInt();

    if (!newToken.isEmpty()){
        settings["iot.token"] = newToken;
        settings["iot.token.refresh_token"] = newRefreshToken;
        settings["iot.token.exp"] = QDateTime::currentMSecsSinceEpoch() + (tokenEpire*1000);
        m_server->writeSettings(settings);
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

    if (connection == 0) return;
    QString url  = connection->getSocket()->requestUrl().path();

}
void WebSocketServer::processMessage(QString message, WebSocketConnection* connection){

    qDebug() << "processTextMessage" << message;

    QRegExp deviceValue("/device/(.+)/value");

    QJsonObject msg =  QJsonDocument::fromJson(message.toLatin1()).object();

    QJsonObject payload = msg.value("payload").toObject();
    int mid = msg.value("mid").toInt(-1);

    QString request =msg.value("name").toString();


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
        response.insert("name", "EventDeviceListUpdate");


        QList<Device*> devices = m_server->getClientList();
        QJsonObject root;
        QString json;
        QJsonArray devs;

        for(int i=0; i<devices.length();i++)
        {
            Device* device = devices.at(i);
            QJsonObject dev;
            dev["name"] = device->getName();
            dev["id"] = device->getID().remove("device:");

            QVariantMap* storedVariables = m_server->getVariablesStorage(device->getID());
            QJsonArray vars;
            for(int i=0; i<device->getVariables()->size(); i++){

                    DeviceVariable* var = device->getVariables()->at(i);
                    QVariantMap res = storedVariables->value(var->getHref()).toMap();

                    QJsonObject v;
                    v["href"] = var->getHref();
                    v["rt"] = var->getResourceType();
                    v["if"] = var->getInterface();
                    v["n"] = var->getName();
                    v["values"]= QJsonObject::fromVariantMap(res);
                    vars.append(v);
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

        Device* device = m_server->getDeviceById(id);
        if (device){
            DeviceVariable* variable = device->getVariable(resource);
            variable->set(value);
        }
    }else if(request == "RequestGetValue"){
        QJsonObject response;
        response.insert("mid",mid);
        QString id = payload.value("di").toString();
        QString resource = payload.value("resource").toString();

        QVariantMap* storedVariables = m_server->getVariablesStorage(id);
        Device* device = m_server->getDeviceById(id);
        if (device){
            DeviceVariable* variable = device->getVariable(resource);
            QVariantMap res = storedVariables->value(variable->getHref()).toMap();

            response.insert("payload", QJsonObject::fromVariantMap(res));
        }
        connection->getSocket()->sendTextMessage(QJsonDocument(response).toJson());

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

    }else if(request == "RequestReloadSchedule"){

        QString id = payload.value("uuid").toString();

        m_server->reloadRule(id);
    }else if(request == "RequestRunScript"){
        qDebug() << "RequestRunScript";

        QString id = payload.value("uuid").toString();
        QJsonObject obj = payload.value("object").toObject();
        m_server->runScriptId(id, obj.toVariantMap());
    }else if(request == "RequestGetDeviceResources"){
        QJsonObject response;
        response.insert("mid",mid);

        QString id = payload.value("uuid").toString();

        Device* dev = m_server->getDeviceById(id);

        QVariantMap* storedVariables = m_server->getVariablesStorage(id);

        QJsonArray vars;
        if (dev)
        {
            for(int i=0; i<dev->getVariables()->size(); i++)
            {
                DeviceVariable* var = dev->getVariables()->at(i);
                QVariantMap res = storedVariables->value(var->getHref()).toMap();

                QJsonObject v;
                v["href"] = var->getHref();
                v["rt"] = var->getResourceType();
                v["if"] = var->getInterface();
                v["n"] = var->getName();
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
    event.insert("name", "EventDeviceListUpdate");

    QList<Device*> devices = m_server->getClientList();
    QJsonObject root;
    QString json;
    QJsonArray devs;

    for(int i=0; i<devices.length();i++)
    {
        Device* device = devices.at(i);
        QJsonObject dev;
        dev["name"] = device->getName();
        dev["id"] = device->getID().remove("device:");

        QVariantMap* storedVariables = m_server->getVariablesStorage(device->getID());
        QJsonArray vars;
        for(int i=0; i<device->getVariables()->size(); i++){

            DeviceVariable* var = device->getVariables()->at(i);
            QVariantMap res = storedVariables->value(var->getHref()).toMap();

            QJsonObject v;
            v["href"] = var->getHref();
            v["rt"] = var->getResourceType();
            v["if"] = var->getInterface();
            v["n"] = var->getName();
            v["values"]= QJsonObject::fromVariantMap(res);
            vars.append(v);
        }
        dev["variables"] = vars;

        devs.append(dev);
    }
    root.insert("devices", devs);
    json = QJsonDocument(root).toJson(QJsonDocument::Compact);

    event.insert("payload", root);

    foreach (WebSocketConnection* s, m_socketList) {
        if (s->isAuthorized())
            s->getSocket()->sendTextMessage(QJsonDocument(event).toJson());

    }
}

void WebSocketServer::onValueChanged(QString id, QString resource, QVariantMap value){
    qDebug() << "WebSocket onValueChanged" << id << resource << value;

    QJsonObject obj;
    obj.insert("name", "EventValueUpdate");

    QJsonObject payload;
    payload.insert("di", id);
    payload.insert("resource", resource);
    payload.insert("value", QJsonObject::fromVariantMap(value));

    obj.insert("payload", payload);

    foreach (WebSocketConnection* s, m_socketList) {
        if (s->isAuthorized() && s->getDeviceSubscription()->contains(id)){
            s->getSocket()->sendTextMessage(QJsonDocument(obj).toJson());
            s->getSocket()->flush();
        }
    }
}


void WebSocketServer::onLogMessage(QString uuid, QString message){
    qDebug() << "onLogMessage" << uuid << message;
    QJsonObject obj;
    obj.insert("name", "EventLog");

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

