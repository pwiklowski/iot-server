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
    qDebug() <<  m_socketServer.listen(QHostAddress::Any, PORT_NUMBER);
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
    QJsonObject requestPayload = req["payload"].toObject();

    qDebug() << requestNamespace << requestName << messageId;

    QString response;
    QJsonObject res;
    QJsonObject responseHeader;
    QJsonObject responsePayload;

    if (requestNamespace == "Alexa.ConnectedHome.Discovery" && requestName == "DiscoverAppliancesRequest"){

        responseHeader["messageId"] = "ff746d98-ab02-4c9e-9d0d-b44711658414";
        responseHeader["name"] = "DiscoverAppliancesResponse";
        responseHeader["namespace"] = "Alexa.ConnectedHome.Discovery";
        responseHeader["payloadVersion"] = "2";

        QJsonArray devices = handleDiscovery();

        responsePayload["discoveredAppliances"] = devices;

        res["header"] = responseHeader;
        res["payload"] = responsePayload;

        response = QJsonDocument(res).toJson();
    }else if(requestNamespace == "Alexa.ConnectedHome.Control"){
        if (handleControl(requestName, requestPayload)){
            responseHeader["messageId"] = "ff746d98-ab02-4c9e-9d0d-b44711658414";

            if (requestName == "TurnOnRequest")
                responseHeader["name"] = "TurnOnConfirmation";

            if (requestName == "TurnOffRequest")
                responseHeader["name"] = "TurnOffConfirmation";

            responseHeader["namespace"] = "Alexa.ConnectedHome.Control";
            responseHeader["payloadVersion"] = "2";

            res["header"] = responseHeader;
            res["payload"] = responsePayload;
            response = QJsonDocument(res).toJson();
        }
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

        if (dev->getVariable("/master") != 0)
        {
            QJsonObject device;

            device["applianceId"] = dev->getID();
            device["manufacturerName"] = "Wiklosoft";
            device["modelName"] = "Wiklosoft Light Controller";
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

        QList<DeviceVariable*>* variables = dev->getVariables();
        foreach (DeviceVariable* var, *variables) {
            if (var->getResourceType() == "oic.r.light.dimming"){
                QJsonObject device;

                device["applianceId"] = dev->getID() + ":" + var->getHref().replace("/","_");
                device["manufacturerName"] = "Wiklosoft";
                device["modelName"] = "Wiklosoft Light Controller";
                device["friendlyName"] = var->getName();
                device["friendlyDescription"] = "OCF resource by Wiklosoft";
                device["isReachable"] = true;
                device["version"] = "0.1";

                QJsonObject additionalApplianceDetails;
                device["additionalApplianceDetails"] = additionalApplianceDetails;

                QJsonArray actions;
                actions.append("setPercentage");
                actions.append("incrementPercentage");
                actions.append("decrementPercentage");
                device["actions"] = actions;

                response.append(device);

            }

        }



    }




    return response;
}


bool AlexaEndpoint::handleControl(QString name, QJsonObject payload){
    QString deviceId = payload["appliance"].toObject()["applianceId"].toString();
    qDebug() << deviceId << name;

    Device* d = m_server->getDeviceById(deviceId);

    if (d == 0) return false;

    DeviceVariable* masterVariable = d->getVariable("/master");

    if (masterVariable == 0) return false;

    QVariantMap m;

    if (name == "TurnOnRequest"){
        m["value"] = true;
        masterVariable->set(m);
    }else if (name == "TurnOffRequest"){
        m["value"] = false;
        masterVariable->set(m);
    }
    return true;
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
