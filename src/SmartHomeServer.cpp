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

#define API_URL "http://127.0.0.1:9000/api"


SmartHomeServer::SmartHomeServer(QObject *parent) :
    QObject(parent)
{
    Settings* settings = new Settings(this);
    m_server.listen(QHostAddress::Any, 9999);
    m_network = new QNetworkAccessManager(this);


    temp = engine.newObject();

}


QByteArray SmartHomeServer::getDeviceScripts(QString id){
    QUrl url(API_URL  "/scripts/device/" + id);

    QNetworkReply *reply = m_network->get(QNetworkRequest(url));

    QEventLoop loop;
    QObject::connect(reply, SIGNAL(readyRead()), &loop, SLOT(quit()));

    loop.exec();

    return reply->readAll();
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
IotDevice *SmartHomeServer::getClient(QString id)
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

QVariant SmartHomeServer::getValue(QString id, QString resource){
    QVariantMap* vars = getVariablesStorage(id);
    if (!vars) return 0;


    return vars->value(resource);
}

bool SmartHomeServer::setValue(QString id, QString resource, QVariantMap value)
{
    IotDevice* client = getClient(id);

    if (client!=0)
    {
        IotDeviceVariable* variable = client->getVariable(resource);

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

QScriptValue logger( QScriptContext * ctx, QScriptEngine * eng ) {
    return QScriptValue();
}
void SmartHomeServer::onValueChanged(QString id, QString resource, QVariantMap value){
    IotDevice* d = getClient(id);

    if (d == 0)
        return;

    qDebug() << "onValueChanged" << d->getID() << resource << value;

    QVariantMap* v = m_variablesStorage.value(d->getID());
    v->insert(resource, value);


    QVariantMap eventInfo;

    QScriptValue event = engine.newObject();
    event.setProperty("source",d->getID());
    event.setProperty("resource", resource);
    event.setProperty("data", engine.toScriptValue(value));

    engine.globalObject().setProperty("event", event);
    engine.globalObject().setProperty("server", engine.newQObject(this));

    QScriptValue time = engine.newObject();

    time.setProperty("minute", QDateTime::currentDateTime().time().minute());
    time.setProperty("hour", QDateTime::currentDateTime().time().hour());
    time.setProperty("second", QDateTime::currentDateTime().time().second());
    time.setProperty("day", QDateTime::currentDateTime().date().day());
    time.setProperty("dayOfWeek", QDateTime::currentDateTime().date().dayOfWeek());
    time.setProperty("month", QDateTime::currentDateTime().date().month());

    engine.globalObject().setProperty("time", time);

    QStringList scripts  = getScripts(d->getID());
    foreach(QString script, scripts)
    {
        QScriptValue error = engine.evaluate(QUrl::fromPercentEncoding(script.toLatin1()));
        qDebug() << "error" << error.toString();
    }
}

