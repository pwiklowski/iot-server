#ifndef SMARTHOMESERVER_H
#define SMARTHOMESERVER_H

#include <QObject>
#include "QTimer"
#include "QTcpServer"
#include "IotEventSetting.h"
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

signals:
    void devicesChanged();
    void valueChanged(QString id, QString resource, QVariantMap value);
    void newLogMessage(QString uuid, QString message);

public slots:

    QByteArray getDeviceScripts(QString id);

    QList<IotDevice *> getClientList();

    QMap<QString, QVariantMap*>* getVariablesStorage() {return &m_variablesStorage;}
    QVariantMap* getVariablesStorage(QString client_id) {return m_variablesStorage.value(client_id);}

    QString getScript(QString id);
    IotDevice *getDeviceByName(QString name);
    IotDevice *getDeviceById(QString id);
    IotDevice *getDeviceByPath(QString path);

    void runScriptId(QString id, QVariantMap obj);
    void runScript(QString scriptId, QString script, QVariantMap obj);

    void onValueChanged(QString id, QString resource, QVariantMap value);

    void deviceAdded(IotDevice* d);
    void deviceRemoved(IotDevice* d);
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

    QList<IotDevice*>  m_clientList;
    QMap<QString, quint8> m_ignoreMap;
    QMap<QString, QVariantMap> m_lastEventMap;
    QJsonArray getScripts(QString src);

    QMap<QString, QVariantMap*> m_variablesStorage;

    QString generateHash();

    QTcpServer m_server;

    QList<IotEventSetting*> mSettingsList;

    QNetworkAccessManager *m_network;

    QString m_token;

    WebSocketServer* m_socketServer;
};

#endif // SMARTHOMESERVER_H
