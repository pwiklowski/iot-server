#ifndef SMARTHOMESERVER_H
#define SMARTHOMESERVER_H

#include <QObject>
#include "QTimer"
#include "QTcpServer"
#include "QtScript/QScriptEngine"
#include "QJsonObject"
#include "QMap"

#include <arpa/inet.h>
#include <net/if.h>

#include "Device.h"
#include "QNetworkAccessManager"
#include "WebSocketServer.h"
#include "qcron.hpp"



class SmartHomeServer : public QObject
{
    Q_OBJECT
public:
    explicit SmartHomeServer(QObject *parent = 0);

    QJsonObject readSettings();
    void writeSettings(QJsonObject data);

signals:
    void devicesChanged();
    void valueChanged(QString id, QString resource, QVariantMap value);
    void newLogMessage(QString uuid, QString message);

public slots:

    QByteArray getDeviceScripts(QString id);

    QList<Device *> getClientList();

    QMap<QString, QVariantMap*>* getVariablesStorage() {return &m_variablesStorage;}
    QVariantMap* getVariablesStorage(QString client_id) {return m_variablesStorage.value(client_id);}

    QString getScript(QString id);
    Device *getDeviceByName(QString name);
    Device *getDeviceById(QString id);
    Device *getDeviceByPath(QString path);
    QList<Device*>* getDevices(){ return &m_clientList; }

    void runScriptId(QString id, QVariantMap obj);
    void runScript(QString scriptId, QString script, QVariantMap obj);

    void onValueChanged(QString id, QString resource, QVariantMap value);

    void deviceAdded(Device *d);
    void deviceRemoved(Device *d);
    void postLog(QString scriptid, QString message);
    bool hasAccess(QString token);

    void runScheduledScript(QString id);

    void initScheduler();
    void reloadRule(QString scriptId);
private:
    QMap<QString, QCron*> m_cronJobs;
    QJsonArray getScripts();
    QJsonObject getScriptData(QString id);
    QScriptValue mapToScriptValue(QMap<QString, QVariant> map);

    QList<Device*>  m_clientList;
    QMap<QString, quint8> m_ignoreMap;
    QMap<QString, QVariantMap> m_lastEventMap;
    QJsonArray getScripts(QString src);

    QMap<QString, QVariantMap*> m_variablesStorage;

    QString generateHash();

    QTcpServer m_server;

    QNetworkAccessManager *m_network;

    QString m_token;

    WebSocketServer* m_socketServer;
};

#endif // SMARTHOMESERVER_H
