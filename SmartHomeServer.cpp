#include "SmartHomeServer.h"
#include "IotEvent.h"
#include "SerialBleScanner.h"
#include "QThread"

#include "QDateTime"


SmartHomeServer::SmartHomeServer(QObject *parent) :
    QObject(parent)
{
    m_server.listen(QHostAddress::Any, 9999);

    connect(&m_server, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));


    //mBleScanner = new BleScanner(this);
    //connect(mBleScanner,SIGNAL(iotEventReceived(QString,QByteArray)), this, SLOT(iotEventReceived(QString,QByteArray)));

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName("/home/pawwik/dev/iot_webui/db.sqlite3");
    m_db.open();

    temp = engine.newObject();


    m_serialScanner = new SerialBleScanner();
    QThread* t = new QThread(this);

    m_serialScanner->moveToThread(t);
    t->start();

    QMetaObject::invokeMethod(m_serialScanner, "read", Qt::QueuedConnection);
    connect(m_serialScanner,SIGNAL(iotEventReceived(QString,QByteArray)), this, SLOT(iotEventReceived(QString,QByteArray)));

}


QStringList SmartHomeServer::getScripts(QString src)
{
    QStringList scripts;

    QSqlQuery query = m_db.exec("SELECT * FROM webui_iotbutton WHERE source='"+src+"';");
    while (query.next()) {
        scripts << query.value(2).toString();
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
    if (eventData.size() <3) return;

    quint8 device_type = eventData.at(1);
    quint8 type = eventData.at(2);
    quint8 event_id = eventData.at(3);

    QVariantMap eventInfo;

    if (m_ignoreMap[source] != event_id)
    {
        m_ignoreMap[source] = event_id;


        if (device_type == IOT_SENSOR)
        {
            if (type == IOT_SENSOR_LIGHT)
            {
                eventInfo.insert("light", (quint8) eventData.at(4));
                eventInfo.insert("type", type);
            }

            m_sensorsMap.insert(source, eventInfo);
        }
        else
        {
            QScriptValue event = engine.newObject();
            event.setProperty("source",source);
            event.setProperty("type",type);


            if (type == IOT_KNOB_WITH_BUTTON)
            {
                eventInfo.insert("button", eventData.at(4));
                eventInfo.insert("knob", eventData.at(5));

            }
            else if (type == IOT_SINGLE_BUTTON)
            {
                eventInfo.insert("pressed", eventData.at(4));
            }

            event.setProperty("data", engine.toScriptValue(eventInfo));

            engine.globalObject().setProperty("event", event);
            engine.globalObject().setProperty("server", engine.newQObject(this));




            engine.globalObject().setProperty("cloud", getGlobalObject(source));


            QScriptValue time = engine.newObject();

            time.setProperty("minute", QDateTime::currentDateTime().time().minute());
            time.setProperty("hour", QDateTime::currentDateTime().time().hour());
            time.setProperty("second", QDateTime::currentDateTime().time().second());
            time.setProperty("day", QDateTime::currentDateTime().date().day());
            time.setProperty("dayOfWeek", QDateTime::currentDateTime().date().dayOfWeek());
            time.setProperty("month", QDateTime::currentDateTime().date().month());

            engine.globalObject().setProperty("time", time);
            QStringList scripts  = getScripts(source);
            foreach(QString script, scripts)
            {
                QScriptValue error = engine.evaluate(script);
                qDebug() << "error" << error.toString();

            }
            saveGlobalObject(source, engine.globalObject().property("cloud"));
            m_lastEventMap.insert(source, eventInfo);

        }


        qDebug() << source << eventInfo;

    }


}
QScriptValue SmartHomeServer::getSensorValue(QString address) {

    QVariantMap value = m_sensorsMap.value(address);

    QScriptValue scriptValue = engine.newObject();


    foreach(QString key, value.keys())
    {
        scriptValue.setProperty(key,engine.toScriptValue(value.value(key)));
    }



    return scriptValue;

}

void SmartHomeServer::handleNewConnection()
{
    qDebug() << "handleNewConnection()";

    QTcpSocket* socket = m_server.nextPendingConnection();

    SmartHomeClient* client = new SmartHomeClient(socket, this);
    connect(client, SIGNAL(disconnected()), this, SLOT(removeClient()));
    connect(this, SIGNAL(clientListChanged()), client, SLOT(deviceListChanged()));

    m_clientList.append(client);
}


QList<SmartHomeClient*> SmartHomeServer::getClientList()
{
    return m_clientList;
}
SmartHomeClient* SmartHomeServer::getClient(QString id)
{
    foreach(SmartHomeClient* client, m_clientList)
    {
        if (client->getID() == id)
        {
            return client;
        }
    }
    return 0;
}

bool SmartHomeServer::setValue(QString id, QString resource, qint32 value)
{
    SmartHomeClient* client = getClient(id);

    if (client!=0)
    {
        client->sendChangeValue(resource, value);
        return true;
    }
    return false;
}

void SmartHomeServer::removeClient()
{
    qDebug() << "removeClient";
    SmartHomeClient* client = (SmartHomeClient*) sender();
    disconnect(this, SIGNAL(clientListChanged()), client, SLOT(deviceListChanged()));

    delete client;
    m_clientList.removeOne(client);
    emit clientListChanged();
}
