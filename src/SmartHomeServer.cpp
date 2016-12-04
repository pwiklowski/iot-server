#include "QUrl"
#include "SmartHomeServer.h"
#include "IotEvent.h"
#include "QThread"
#include "Settings.h"
#include "QDateTime"

#include "QNetworkRequest"
#include "QNetworkReply"
#include "QTimer"
#include "QEventLoop"
#include "QJsonDocument"
#include "QJsonArray"
#include "QJsonValue"
#include "QVariantMap"

#define API_URL "http://127.0.0.1:7000/api"


SmartHomeServer::SmartHomeServer(QObject *parent) :
    QObject(parent)
{
    Settings* settings = new Settings(this);
    m_server.listen(QHostAddress::Any, 9999);
    m_network = new QNetworkAccessManager(this);

    temp = engine.newObject();

}


QScriptEngine* SmartHomeServer::getEngine(){
    return &engine;
}

bool SmartHomeServer::hasAccess(QString token){
    QUrl url(API_URL  "/websocketaccess");

    QNetworkRequest req(url);
    req.setRawHeader(QString("Authorization").toLatin1(), token.toLatin1());


    QNetworkReply *reply = m_network->get(req);

    QEventLoop loop;
    QObject::connect(reply, SIGNAL(readyRead()), &loop, SLOT(quit()));

    loop.exec();

    return reply->error() == QNetworkReply::NoError;
}


QString SmartHomeServer::getScript(QString id){
    QUrl url(API_URL  "/script/" + id);
    QNetworkReply *reply = m_network->get(QNetworkRequest(url));

    QEventLoop loop;
    QObject::connect(reply, SIGNAL(readyRead()), &loop, SLOT(quit()));

    loop.exec();

    QJsonDocument response = QJsonDocument::fromJson(reply->readAll());

    QJsonValue s = response.object().take("Scripts").toArray().at(0).toObject().take("Content");

    QString script = QByteArray::fromBase64(s.toString().toLatin1());

    return script;
}

void SmartHomeServer::postLog(QString scriptid, QString message){
    emit newLogMessage(scriptid, message);
}


QByteArray SmartHomeServer::getDeviceScripts(QString id){
    QUrl url(API_URL  "/scripts/device/" + id);
    QNetworkReply *reply = m_network->get(QNetworkRequest(url));

    QEventLoop loop;
    QObject::connect(reply, SIGNAL(readyRead()), &loop, SLOT(quit()));

    loop.exec();

    QByteArray data = reply->readAll();

    delete reply;

    return data;
}

void SmartHomeServer::deviceAdded(IotDevice* d){
   m_variablesStorage.insert(d->getID(), new QVariantMap());
   m_clientList.append(d);
   emit devicesChanged();
}

void SmartHomeServer::deviceRemoved(IotDevice *d){
    m_variablesStorage.remove(d->getID());
    m_clientList.removeOne(d);
    emit devicesChanged();
}

QList<IotDevice *> SmartHomeServer::getClientList()
{
    return m_clientList;
}
IotDevice *SmartHomeServer::getDeviceByName(QString name)
{
    foreach(IotDevice* client, m_clientList)
    {
        if (client->getName() == name)
        {
            return client;
        }
    }
    return 0;
}

IotDevice *SmartHomeServer::getDeviceById(QString id)
{
    foreach(IotDevice* client, m_clientList)
    {
        if (client->getID() == id)
        {
            return client;
        }
    }
    return 0;
}

IotDevice *SmartHomeServer::getDeviceByPath(QString path)
{
    QStringList p = path.mid(0, path.indexOf("/")).split(":");

    if (p.at(0) == "id")
        return getDeviceById(p.at(1));
    else
        return getDeviceByName(p.at(1));

}

QVariant SmartHomeServer::getValue(QString resource){
    IotDevice* dev = getDeviceByPath(resource);


    QVariantMap* vars = getVariablesStorage(dev->getID());
    if (!vars) return 0;


    return vars->value(resource);
}



bool SmartHomeServer::setValue(QString resource, QVariantMap value)
{
    qDebug() << resource << value;
    IotDevice* client = getDeviceByPath(resource);

    QString variableURI= resource.mid(resource.indexOf("/"));

    if (client!=0)
    {
        IotDeviceVariable* variable = client->getVariable(variableURI);

        if (variable != 0){
            variable->set(value);

        }
        return true;
    }
    return false;
}
QJsonArray SmartHomeServer::getScripts(QString id)
{
    QStringList scripts;

    QString responseJson = getDeviceScripts(id);

    QJsonDocument response = QJsonDocument::fromJson(responseJson.toLatin1());
    return response.array();
}
void SmartHomeServer::saveGlobalObject(QString key, QScriptValue obj)
{
    m_cloudScriptStorage.insert(key, obj);
}
QScriptValue SmartHomeServer::getGlobalObject(QString key)
{
    if (!m_cloudScriptStorage.keys().contains(key))
        return engine.newObject();
    return m_cloudScriptStorage.value(key);
}


void SmartHomeServer::iotEventReceived(QString source,  QByteArray eventData)
{

}

void SmartHomeServer::debug(QString str ) {
    qDebug() << str;
    postLog("fcb05237-08c6-4c07-8b9f-c243c558f4fa", str);
}


void SmartHomeServer::runScriptId(QString id, QVariantMap obj){
    QString script = getScript(id);
    runScript(id, script, obj);
}

void SmartHomeServer::runScript(QString scriptId, QString script, QVariantMap obj){
    postLog(scriptId, "Start script " + scriptId);
    //postLog(script, "event: " + obj.));
    QScriptValue event = engine.newObject();

    foreach(QVariant key, obj.keys()){
       event.setProperty(key.toString(), engine.newVariant(obj.value(key.toString())));
    }


    engine.globalObject().setProperty("Event", event);
    engine.globalObject().setProperty("Server", engine.newQObject(this));

    QScriptValue time = engine.newObject();

    time.setProperty("minute", QDateTime::currentDateTime().time().minute());
    time.setProperty("hour", QDateTime::currentDateTime().time().hour());
    time.setProperty("second", QDateTime::currentDateTime().time().second());
    time.setProperty("day", QDateTime::currentDateTime().date().day());
    time.setProperty("dayOfWeek", QDateTime::currentDateTime().date().dayOfWeek());
    time.setProperty("month", QDateTime::currentDateTime().date().month());

    engine.globalObject().setProperty("time", time);

    QScriptValue storage = m_cloudScriptStorage.value(scriptId, engine.newObject());
    engine.globalObject().setProperty("ctx", storage);

    QScriptValue error = engine.evaluate(script);

    if (error.toString() == "undefined"){
        postLog(scriptId, "Success");
    }else{
        postLog(scriptId, error.toString());
        qDebug() << "error" << error.toString();
    }


    postLog(scriptId, "End script " + scriptId);
    m_cloudScriptStorage.insert(scriptId, storage);
}


void SmartHomeServer::onValueChanged(QString id, QString resource, QVariantMap value){
    IotDevice* d = getDeviceById(id);

    if (d == 0)
        return;

    QVariantMap event;

    event["source"] = d->getID();
    event["resource"] = resource;
    event["value"] = value;


    QJsonArray scripts  = getScripts(d->getID());

    foreach(QJsonValue obj, scripts){
        QJsonValue s = obj.toObject().take("Scripts").toArray().at(0).toObject().take("Content");
        QJsonValue scriptId = obj.toObject().take("ScriptUuid").toString();
        QString script = QByteArray::fromBase64(s.toString().toLatin1());

        runScript(scriptId.toString(), script, event);
    }



    QVariantMap* vars = getVariablesStorage(d->getID());

    vars->insert(resource, (QVariantMap) value);


    emit valueChanged(id, resource, value);
}

