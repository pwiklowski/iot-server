#include "AlexaEndpoint.h"
#include "QTcpSocket"
#include "QJsonObject"
#include "QJsonArray"
#include "QJsonDocument"

#define PORT_NUMBER 12345

#define NAMESPACE_CONTROL "Alexa.ConnectedHome.Control"
#define NAMESPACE_DISCOVERY "Alexa.ConnectedHome.Discovery"

#define DISCOVER_APPLIANCES_REQUEST "DiscoverAppliancesRequest"
#define DISCOVER_APPLIANCES_RESPONSE "DiscoverAppliancesResponse"


#define TURN_ON_REQUEST "TurnOnRequest"
#define TURN_OFF_REQUEST "TurnOffRequest"
#define TURN_ON_CONFIRMATION "TurnOnConfirmation"
#define TURN_OFF_CONFIRMATION "TurnOffConfirmation"


#define SET_PERCENTAGE_REQUEST "SetPercentageRequest"
#define SET_PERCENTAGE_CONFIRMATION "SetPercentageConfirmation"
#define INCREMENT_PERCENTAGE_REQUEST "IncrementPercentageRequest"
#define INCREMENT_PERCENTAGE_CONFIRMATION "IncrementPercentageConfirmation"
#define DECREMENT_PERCENTAGE_REQUEST "DecrementPercentageRequest"
#define DECREMENT_PERCENTAGE_CONFIRMATION "DecrementPercentageConfirmation"


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

    if (requestNamespace == NAMESPACE_DISCOVERY && requestName == DISCOVER_APPLIANCES_REQUEST){

        responseHeader["messageId"] = "ff746d98-ab02-4c9e-9d0d-b44711658414";
        responseHeader["name"] = DISCOVER_APPLIANCES_RESPONSE;
        responseHeader["namespace"] = NAMESPACE_DISCOVERY;
        responseHeader["payloadVersion"] = "2";

        QJsonArray devices = handleDiscovery();

        responsePayload["discoveredAppliances"] = devices;

        res["header"] = responseHeader;
        res["payload"] = responsePayload;

        response = QJsonDocument(res).toJson();
    }else if(requestNamespace == NAMESPACE_CONTROL){
        handleControl(requestName, requestPayload, &response);
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
            if (var->getResourceType() == RT_OIC_R_LIGHT_DIMMING){
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


bool AlexaEndpoint::handleControl(QString requestName, QJsonObject payload, QString* response){
    QJsonObject res;
    QJsonObject responseHeader;
    QJsonObject responsePayload;

    responseHeader["messageId"] = "ff746d98-ab02-4c9e-9d0d-b44711658414"; //TODO: generate message ID
    responseHeader["namespace"] = NAMESPACE_CONTROL;
    responseHeader["payloadVersion"] = "2";


    QString applicanceId = payload["appliance"].toObject()["applianceId"].toString();
    qDebug() << applicanceId << requestName;

    QStringList applicanceIds = applicanceId.split(":");

    QString deviceId = applicanceIds.at(0);
    QString resource;
    if (applicanceIds.length() ==2){
        resource = applicanceIds.at(1);
    }

    if (requestName == TURN_ON_REQUEST || requestName == TURN_OFF_REQUEST){
        responseHeader["name"] = requestName == TURN_ON_REQUEST ? TURN_ON_CONFIRMATION : TURN_OFF_CONFIRMATION;
        onTurnOnOffRequest(applicanceId, requestName == TURN_ON_REQUEST);
    }else if(requestName == SET_PERCENTAGE_REQUEST){
        responseHeader["name"] = SET_PERCENTAGE_CONFIRMATION;
        quint8 percent = payload["percentageState"].toObject()["value"].toInt();

        onSetPercentageRequest(deviceId, resource.replace("_", "/"), percent);
    }else if(requestName == INCREMENT_PERCENTAGE_REQUEST){
        responseHeader["name"] = INCREMENT_PERCENTAGE_CONFIRMATION;
        quint8 percent = payload["deltaPercentage"].toObject()["value"].toInt();
        onIncreasePercentageRequest(deviceId, resource.replace("_", "/"), percent);
    }else if(requestName == DECREMENT_PERCENTAGE_REQUEST){
        responseHeader["name"] = DECREMENT_PERCENTAGE_CONFIRMATION;
        quint8 percent = payload["deltaPercentage"].toObject()["value"].toInt();
        onIncreasePercentageRequest(deviceId, resource.replace("_", "/"), -percent);
    }


    res["header"] = responseHeader;
    res["payload"] = responsePayload;
    *response = QJsonDocument(res).toJson();
}

void AlexaEndpoint::onIncreasePercentageRequest(QString deviceId, QString resource, qint8 percent){
    qDebug() << "onIncreasePercentageRequest" << deviceId << resource << percent;

    Device* device = m_server->getDeviceById(deviceId);
    if (device == 0) return;
    DeviceVariable* variable = device->getVariable(resource);
    if (variable == 0) return;

    if (variable->getResourceType() == RT_OIC_R_LIGHT_DIMMING){
        QVariantMap* vars = m_server->getVariablesStorage(deviceId);
        QString range = vars->value(resource).toMap()["range"].toString();
        qDebug() << *vars;
        qint16 dimmingSetting = vars->value(resource).toMap().value("dimmingSetting").toInt();

        if (range.isEmpty()) range = "0,255";

        QString max = range.split(",").at(1);

        qint16 value = max.toInt()* percent /100;

        qDebug() << "onIncreasePercentageRequest from:"<< dimmingSetting << "to:" << dimmingSetting+value;
        dimmingSetting = dimmingSetting + value;

        if (dimmingSetting > max.toInt()) dimmingSetting = max.toInt();
        if (dimmingSetting < 0) dimmingSetting = 0;


        QVariantMap m;
        m["dimmingSetting"] = dimmingSetting;
        variable->set(m);
    }
}

void AlexaEndpoint::onSetPercentageRequest(QString deviceId, QString resource, quint8 percent){
    qDebug() << "onSetPercentRequest" << deviceId << resource << percent;

    Device* device = m_server->getDeviceById(deviceId);
    DeviceVariable* variable = device->getVariable(resource);

    if (variable == 0) return;

    if (variable->getResourceType() == RT_OIC_R_LIGHT_DIMMING){
        QVariantMap* vars = m_server->getVariablesStorage(deviceId);
        QString range = vars->value("range").toString();

        if (range.isEmpty()) range = "0,255";

        QString max = range.split(",").at(1);

        quint16 value = max.toInt()* percent /100;

        QVariantMap m;
        m["dimmingSetting"] = value;
        variable->set(m);
    }

}

void AlexaEndpoint::onTurnOnOffRequest(QString deviceId, bool isTurnOn){
    Device* d = m_server->getDeviceById(deviceId);
    if (d == 0) return;
    DeviceVariable* masterVariable = d->getVariable("/master");
    if (masterVariable == 0) return;
    QVariantMap m;
    m["value"] = isTurnOn;
    masterVariable->set(m);
}

