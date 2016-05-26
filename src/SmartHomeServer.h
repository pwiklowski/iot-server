#ifndef SMARTHOMESERVER_H
#define SMARTHOMESERVER_H

#include <QObject>
#include "QTimer"
#include "QTcpServer"
#include "IotEventSetting.h"
#include "QtScript/QScriptEngine"
#include "QMap"
#include "QSqlDatabase"
#include "QSqlQuery"
#include "QSqlError"

#include <arpa/inet.h>
#include <net/if.h>

#include "Device.h"



class SmartHomeServer : public QObject
{
    Q_OBJECT
public:
    explicit SmartHomeServer(QObject *parent = 0);
    static void* run(void* param);

      static void* waitForDevices(void* param);
    OICClient* getClient(){return m_client;}
    void setSocketFd(int s) { m_socketFd = s;}
signals:
    void devicesChanged();
public slots:
    void ping();

    QList<Device*> getClientList();
    void iotEventReceived(QString source,  QByteArray eventData);
    QVariant getValue(QString id, QString resource){return getVariablesStorage(id)->value(resource);}
    bool setValue(QString id, QString resource, QVariantMap value);

    QMap<QString, QVariantMap> getLastEventMap() { return m_lastEventMap;}
    QMap<QString, QVariantMap*>* getVariablesStorage() {return &m_variablesStorage;}
    QVariantMap* getVariablesStorage(QString client_id) {return m_variablesStorage.value(client_id);}
    void findDevices();


    Device* getClient(QString id);

    void saveGlobalObject(QString key, QScriptValue obj);
    QScriptValue getGlobalObject(QString key);

    void onValueChanged(QString resource, QVariantMap value);
private:
    QTimer m_timer;
    QTimer m_deviceSearchTimer;
    String convertAddress(sockaddr_in a);
    bool isDeviceOnList(QString id);


    void send_packet(sockaddr_in destination, COAPPacket* packet);
    void send_packet(COAPPacket* packet);
    OICClient* m_client;
    QObject* m_parent;
    pthread_t m_thread;
    pthread_t m_deviceThread;
    int m_socketFd;

    QMap<QString, quint8> m_ignoreMap;
    QMap<QString, QVariantMap> m_lastEventMap;
    QMap<QString, QVariantMap> m_sensorsMap;
    QStringList getScripts(QString src);
    QScriptValue temp;
    QScriptEngine engine;

    QMap<QString, QVariantMap*> m_variablesStorage;
    QMap<QString,QScriptValue> m_cloudScriptStorage;

    QString generateHash();

    QSqlDatabase m_db;
    QTcpServer m_server;
    QList<Device*>  m_clientList;
    QList<IotEventSetting*> mSettingsList;
};

#endif // SMARTHOMESERVER_H
