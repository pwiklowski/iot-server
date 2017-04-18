#include "AlexaEndpoint.h"
#include "QTcpSocket"
#include "QJsonObject"
#include "QJsonArray"
#include "QJsonDocument"

#define PORT_NUMBER 12345

AlexaEndpoint::AlexaEndpoint(SmartHomeServer* server, QObject *parent) : QObject(parent)
{
    m_server = server;
    connect(&m_socketServer, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));
    m_socketServer.listen(QHostAddress::Any, PORT_NUMBER);
}


void AlexaEndpoint::handleNewConnection(){
    qDebug() << "handleNewConnection";

    QTcpSocket* socket = m_socketServer.nextPendingConnection();
    connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));

    socket->waitForReadyRead();
    QString request = socket->readAll();
    //qDebug() << request;
    QString payload = request.split("\r\n\r\n").at(1);
    qDebug() << payload;
    QJsonObject req =  QJsonDocument::fromJson(payload.toLatin1()).object();

    QString requestNamespace = req["header"].toObject()["namespace"].toString();
    QString requestName = req["header"].toObject()["name"].toString();
    QString messageId = req["header"].toObject()["name"].toString();

    qDebug() << requestNamespace << requestName << messageId;

    QString response;
    if (requestNamespace == "Alexa.ConnectedHome.Discovery" && requestName == "DiscoverAppliancesRequest"){
        QJsonObject res;

        QJsonObject header;
        QJsonObject payload;

        header["messageId"] = "ff746d98-ab02-4c9e-9d0d-b44711658414";
        header["name"] = "DiscoverAppliancesResponse";
        header["namespace"] = "Alexa.ConnectedHome.Discovery";
        header["payloadVersion"] = "2";

        QJsonArray devices = handleDiscovery();

        payload["discoveredAppliances"] = devices;

        res["header"] = header;
        res["payload"] = payload;

        response = QJsonDocument(res).toJson();
    }else{
        QJsonObject res = handleCommand(req);
        response = QJsonDocument(res).toJson();
    }

    socket->write("HTTP/1.1 200 OK\r\n");
    socket->write("Server: Qt Web Server\r\n");
    socket->write("Content-Type: text/html; charset=utf-8\r\n");
    QString responseLength = "Content-Length: " + QString::number(response.length()) + "\r\n";
    socket->write(responseLength.toUtf8());
    socket->write("Connection: close\r\n");
    socket->write("\r\n");
    socket->write(response.toUtf8());
    socket->disconnectFromHost();

    qDebug () << response;
}

QJsonArray AlexaEndpoint::handleDiscovery(){
    QJsonArray response;

    QList<Device*>* devices = m_server->getDevices();

    foreach (Device* dev, *devices) {
        QJsonObject device;
        device["applianceId"] = dev->getID();
        device["manufacturerName"] = "Wiklosoft";
        device["modelName"] = "Wiklosfot Light Controller";
        device["version"] = "1";
        device["friendlyName"] = dev->getName();
        device["friendlyDescription"] = "Switch by Sample Manufacturer";
        device["isReachable"] = true;
        device["version"] = "0.1";

        QJsonObject additionalApplianceDetails;
        device["additionalApplianceDetails"] = additionalApplianceDetails;

        QJsonArray actions;
        actions.append("turnOn");
        actions.append("turnOff");

        device["actions"] = actions;

        response.append(device);
    }


    return response;
}


QJsonObject AlexaEndpoint::handleCommand(QJsonObject request){
    QJsonObject response;
    response["version"] = "1.0";

    QString requestType = request["request"].toObject()["type"].toString();
    QJsonObject intent = request["request"].toObject()["intent"].toObject();
    QString requestIntent = intent["name"].toString();

    qDebug() << requestType << requestIntent << intent;


    handleIntent(intent);

    QJsonObject outputSpeech;
    outputSpeech["type"] = "PlainText";
    outputSpeech["text"] = "yey!";


    QJsonObject res;
    res["shouldEndSession"] = true;
    res["outputSpeech"] = outputSpeech;


    response["response"] = res;
    return response;
}


void AlexaEndpoint::handleIntent(QJsonObject intent){
    QString name = intent["name"].toString();

    QJsonObject slot = intent["slots"].toObject();

    if (name == "SetDeviceResourcesValueIntent"){
        QString resource = slot["Resource"].toObject()["value"].toString();
        quint16 value = slot["Value"].toObject()["value"].toString().toInt();

        qDebug() << "set" << resource << value;

        Device* d = m_server->getDeviceById("00000000-0000-0000-0001-000000000001");

        QVariantMap m;
        m["dimmingSetting"] = value;

        DeviceVariable* var = d->getVariable("/lampa/"+resource);
        if (var != 0){
            var->set(m);
        }
    }


}
