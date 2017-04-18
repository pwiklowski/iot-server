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
signals:

public slots:
    void handleNewConnection();
private:
    QTcpServer m_socketServer;
    SmartHomeServer* m_server;

};

#endif // ALEXAENDPOINT_H
