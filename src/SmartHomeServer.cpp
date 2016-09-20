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
#include "QJsonObject"
#include "QVariantMap"

#define API_URL "http://127.0.0.1:9000/api"


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
    QUrl url(API_URL  "/logs/" + scriptid);
    QNetworkReply *reply = m_network->post(QNetworkRequest(url), message.toLatin1());

    QEventLoop loop;
    QObject::connect(reply, SIGNAL(readyRead()), &loop, SLOT(quit()));

    loop.exec();
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
}

void SmartHomeServer::deviceRemoved(IotDevice *d){
    m_variablesStorage.remove(d->getID());
    m_clientList.removeOne(d);

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
QStringList SmartHomeServer::getScripts(QString id)
{
    QStringList scripts;

    QString responseJson = getDeviceScripts(id);

    QJsonDocument response = QJsonDocument::fromJson(responseJson.toLatin1());
    QJsonArray array = response.array();

    foreach(QJsonValue obj, array){
        QJsonValue s = obj.toObject().take("Scripts").toArray().at(0).toObject().take("Content");
        scripts.append(QByteArray::fromBase64(s.toString().toLatin1()));
    }

    return scripts;
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



void SmartHomeServer::runScript(QString id, QScriptValue event){
    QString script = getScript(id);
    postLog(id, "Run script " + id + event.toString());

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

    QScriptValue error = engine.evaluate(script);

    if (error.toString() == "undefined"){
        postLog(id, "Success");
    }else{
        postLog(id, error.toString());
        qDebug() << "error" << error.toString();
    }





}


void SmartHomeServer::onValueChanged(QString id, QString resource, QVariantMap value){
    IotDevice* d = getDeviceById(id);

    if (d == 0)
        return;

    QVariantMap event;

    event["source"] = d->getID();
    event["resource"] = resource;
    event["value"] = value;


    QStringList scripts  = getScripts(d->getID());
    foreach(QString script, scripts)
    {


    }


    QVariantMap* vars = getVariablesStorage(d->getID());

    vars->insert(resource, (QVariantMap) value);


    emit valueChanged(id, resource, value);
}

