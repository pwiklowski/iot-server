#ifndef SMARTHOMESERVER_H
#define SMARTHOMESERVER_H

#include <QObject>
#include "QTcpServer"
#include "SmartHomeClient.h"
#include "IotEventSetting.h"
#include "QtScript/QScriptEngine"
#include "QMap"
#include "QSqlDatabase"
#include "QSqlQuery"
#include "QSqlError"
#include "SerialBleScanner.h"

enum IotEventSource
{
    IOT_SINGLE_BUTTON = 2,
    IOT_KNOB_WITH_BUTTON = 1,
};

enum IotDeviceType
{
    IOT_BUTTON = 2,
    IOT_SENSOR = 3,

};

enum IotSensorType
{
    IOT_SENSOR_LIGHT = 0,

};



class SmartHomeServer : public QObject
{
    Q_OBJECT
public:
    explicit SmartHomeServer(QObject *parent = 0);

signals:
    void clientListChanged();
public slots:
    void handleNewConnection();
    QList<SmartHomeClient*> getClientList();
    void removeClient();
    void iotEventReceived(QString source,  QByteArray eventData);
    quint16 getValue(QString id, QString resource){return getVariablesStorage(id)->value(resource).toInt();}
    bool setValue(QString id, QString resource, qint32 value);
    QScriptValue getSensorValue(QString address);
    QMap<QString, QVariantMap> getSensorsMap() { return m_sensorsMap;}

    QMap<QString, QVariantMap> getLastEventMap() { return m_lastEventMap;}
    QMap<QString, QVariantMap*>* getVariablesStorage() {return &m_variablesStorage;}
    QVariantMap* getVariablesStorage(QString client_id) {return m_variablesStorage.value(client_id);}


    SmartHomeClient* getClient(QString id);

    void saveGlobalObject(QString key, QScriptValue obj);
    QScriptValue getGlobalObject(QString key);

private:
    QMap<QString, quint8> m_ignoreMap;
    QMap<QString, QVariantMap> m_lastEventMap;
    QMap<QString, QVariantMap> m_sensorsMap;
    QStringList getScripts(QString src);
     QScriptValue temp;
     QScriptEngine engine;

    SerialBleScanner* m_serialScanner;
    QMap<QString, QVariantMap*> m_variablesStorage;
    QMap<QString,QScriptValue> m_cloudScriptStorage;

    QString generateHash();

    QSqlDatabase m_db;
    QTcpServer m_server;
    QList<SmartHomeClient*>  m_clientList;
    QList<IotEventSetting*> mSettingsList;
};

#endif // SMARTHOMESERVER_H
