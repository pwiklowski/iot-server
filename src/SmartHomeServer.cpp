#include "QUrl"
#include "SmartHomeServer.h"
#include "IotEvent.h"
#include "QThread"
#include "Settings.h"
#include "QDateTime"

#include "QTimer"

SmartHomeServer::SmartHomeServer(QObject *parent) :
    QObject(parent)
{
    Settings* settings = new Settings(this);
    m_server.listen(QHostAddress::Any, 9999);

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setConnectOptions("QSQLITE_OPEN_READONLY");
    //m_db.setDatabaseName(settings->getValue("database").toString());
    m_db.setDatabaseName("/home/pawwik/dev/iot_webui/db.sqlite3");

    if (!m_db.open())
    {
        qWarning() << "Unable to open database";
    }

    temp = engine.newObject();
}

void SmartHomeServer::deviceAdded(Device* d){
   m_variablesStorage.insert(d->getID(), new QVariantMap());
   m_clientList.append(d);
}

void SmartHomeServer::deviceRemoved(Device* d){
    m_variablesStorage.remove(d->getID());
    m_clientList.removeOne(d);

}

QList<Device *> SmartHomeServer::getClientList()
{
    return m_clientList;
}
Device *SmartHomeServer::getClient(QString id)
{
    foreach(Device* client, m_clientList)
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
    Device* client = getClient(id);

    if (client!=0)
    {
        DeviceVariable* variable = client->getVariable(resource);

        if (variable != 0){
            variable->post(value);

        }


        return true;
    }
    return false;
}
QStringList SmartHomeServer::getScripts(QString id)
{
    QStringList scripts;

    QSqlQuery query = m_db.exec("SELECT script FROM webui_iotscripts WHERE di='"+id+"';");
    qDebug() << query.lastError().text();
    while (query.next()) {
        scripts << query.value(0).toString();
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
    Device* d = getClient(id);

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

