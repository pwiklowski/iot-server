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
#include "ScriptRunner.h"

#include "QProcess"


#define API_URL "http://127.0.0.1:9000/api"


SmartHomeServer::SmartHomeServer(QObject *parent) :
    QObject(parent)
{
    Settings* settings = new Settings(this);
    m_server.listen(QHostAddress::Any, 9999);
    m_network = new QNetworkAccessManager(this);

    m_socketServer = new WebSocketServer(this);
    initScheduler();
}


bool SmartHomeServer::hasAccess(QString token){
    QUrl url(API_URL  "/websocketaccess");

    QNetworkRequest req(url);
    req.setRawHeader(QString("Authorization").toLatin1(), token.toLatin1());


    QNetworkReply *reply = m_network->get(req);

    QEventLoop loop;
    QObject::connect(reply, SIGNAL(readyRead()), &loop, SLOT(quit()));

    loop.exec();

    if (reply->error() == QNetworkReply::NoError){
        m_token = token;
    }

    return reply->error() == QNetworkReply::NoError;
}

void SmartHomeServer::initScheduler(){

    QJsonArray scripts = getScripts();

    for (quint16 i; i<scripts.size(); i++){
        QString id = scripts.at(i).toObject().take("Id").toString();
        QString schedule = scripts.at(i).toObject().take("Schedule").toString();

        if (!schedule.isEmpty()){
            qDebug() << "Add cron job" << scripts.at(i).toObject().take("Name").toString() << scripts.at(i).toObject().take("Schedule").toString();
            QCron* job = new QCron(id, schedule);
            connect(job , SIGNAL(activated(QString)), this, SLOT(runScheduledScript(QString)));
            m_cronJobs.insert(id ,job);
        }
    }

}

void SmartHomeServer::runScheduledScript(QString id){
    qDebug() << "runScheduledScript" << id << QDateTime::currentDateTime();



}
QJsonArray SmartHomeServer::getScripts(){
    QUrl url(API_URL  "/scripts");

    QNetworkRequest req(url);
    req.setRawHeader(QString("Authorization").toLatin1(), m_token.toLatin1());

    QNetworkReply *reply = m_network->get(req);

    QEventLoop loop;
    QObject::connect(reply, SIGNAL(readyRead()), &loop, SLOT(quit()));

    loop.exec();

    QByteArray res = reply->readAll();
    qDebug() << res;
    QJsonDocument response = QJsonDocument::fromJson(res);


    return response.array();
}


QString SmartHomeServer::getScript(QString id){
    QUrl url(API_URL  "/script/" + id);

    QNetworkRequest req(url);
    req.setRawHeader(QString("Authorization").toLatin1(), m_token.toLatin1());

    QNetworkReply *reply = m_network->get(req);

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
    QNetworkRequest req(url);
    req.setRawHeader(QString("Authorization").toLatin1(), m_token.toLatin1());

    QNetworkReply *reply = m_network->get(req);

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

QJsonArray SmartHomeServer::getScripts(QString id)
{
    QStringList scripts;

    QString responseJson = getDeviceScripts(id);

    QJsonDocument response = QJsonDocument::fromJson(responseJson.toLatin1());
    return response.array();
}

void SmartHomeServer::runScriptId(QString id, QVariantMap obj){
    QString script = getScript(id);
    runScript(id, script, obj);
}

void SmartHomeServer::runScript(QString scriptId, QString script, QVariantMap obj){
    ScriptRunner* sr = new ScriptRunner(m_socketServer, scriptId, script);
    sr->start(obj);
    connect(sr, SIGNAL(finished()), sr, SLOT(deleteLater()));

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

