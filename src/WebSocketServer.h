#ifndef WEBSOCKETSERVER_H
#define WEBSOCKETSERVER_H

#include <QObject>
#include "QtWebSockets/qwebsocketserver.h"
#include "QtWebSockets/qwebsocket.h"

#include "WebSocketConnection.h"

class SmartHomeServer;


class WebSocketServer : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketServer(SmartHomeServer *parent = 0);

signals:

public slots:
    void onNewConnection();
    void processTextMessage(QString message);
    void processMessage(QString message, WebSocketConnection *connection);
    void socketDisconnected();
    void onValueChanged(QString id, QString resource, QVariantMap value);
    void onDeviceListUpdate();
    void onLogMessage(QString uuid, QString message);

private:
    WebSocketConnection* getSocketConnection(QWebSocket* socket);
    SmartHomeServer* m_server;
    QWebSocketServer* m_webSocketServer;
    QList<WebSocketConnection*> m_socketList;

    WebSocketConnection* m_iotCloudConnection;
};

#endif // WEBSOCKETSERVER_H
