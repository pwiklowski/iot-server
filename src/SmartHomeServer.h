#ifndef SMARTHOMESERVER_H
#define SMARTHOMESERVER_H

#include <QObject>
#include "QTimer"
#include "QTcpServer"
#include "IotEventSetting.h"
#include "QtScript/QScriptEngine"
#include "QMap"

#include <arpa/inet.h>
#include <net/if.h>

#include "Device.h"
#include "QNetworkAccessManager"


class SmartHomeServer : public QObject
{
    Q_OBJECT
public:
    explicit SmartHomeServer(QObject *parent = 0);

signals:
    void devicesChanged();
public slots:

    QByteArray getDeviceScripts(QString id);

    QList<IotDevice *> getClientList();
    void iotEventReceived(QString source,  QByteArray eventData);
    QVariant getValue(QString resource);
    bool setValue(QString resource, QVariantMap value);

    QMap<QString, QVariantMap> getLastEventMap() { return m_lastEventMap;}
    QMap<QString, QVariantMap*>* getVariablesStorage() {return &m_variablesStorage;}
    QVariantMap* getVariablesStorage(QString client_id) {return m_variablesStorage.value(client_id);}




    QString getScript(QString id);
    IotDevice *getDeviceByName(QString name);
    IotDevice *getDeviceById(QString id);
    IotDevice *getDeviceByPath(QString path);


    void runScript(QString id, QVariant event);

    void saveGlobalObject(QString key, QScriptValue obj);
    QScriptValue getGlobalObject(QString key);

    void onValueChanged(QString id, QString resource, QVariantMap value);

    void deviceAdded(IotDevice* d);
    void deviceRemoved(IotDevice* d);
private:
    QList<IotDevice*>  m_clientList;
    QMap<QString, quint8> m_ignoreMap;
    QMap<QString, QVariantMap> m_lastEventMap;
    QMap<QString, QVariantMap> m_sensorsMap;
    QStringList getScripts(QString src);
    QScriptValue temp;
    QScriptEngine engine;

    QMap<QString, QVariantMap*> m_variablesStorage;
    QMap<QString,QScriptValue> m_cloudScriptStorage;

    QString generateHash();

    QTcpServer m_server;

    QList<IotEventSetting*> mSettingsList;

    QNetworkAccessManager *m_network;
};

#endif // SMARTHOMESERVER_H
