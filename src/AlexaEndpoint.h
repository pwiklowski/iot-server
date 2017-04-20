#ifndef ALEXAENDPOINT_H
#define ALEXAENDPOINT_H

#include <QObject>
#include "QTcpServer"
#include "SmartHomeServer.h"

class AlexaEndpoint : public QObject
{
    Q_OBJECT
public:
    explicit AlexaEndpoint(SmartHomeServer* server, QObject *parent = 0);

    QJsonObject handleCommand(QJsonObject request);
    void handleIntent(QJsonObject intent);
    QJsonArray handleDiscovery();
    bool handleControl(QString requestName, QJsonObject payload, QString *response);

    void onSetPercentageRequest(QString deviceId, QString resource, quint8 percent);
    void onIncreasePercentageRequest(QString deviceId, QString resource, quint8 percent);
    void onDecreasePercentageRequest(QString deviceId, QString resource, quint8 percent);
    void onTurnOnOffRequest(QString deviceId, bool isTurnOn);
signals:

public slots:
    void handleNewConnection();
private:
    QTcpServer m_socketServer;
    SmartHomeServer* m_server;

};

#endif // ALEXAENDPOINT_H
