#ifndef WEBSOCKETSERVER_H
#define WEBSOCKETSERVER_H

#include "SmartHomeServer.h"
#include <QObject>
#include "QtWebSockets/qwebsocketserver.h"
#include "QtWebSockets/qwebsocket.h"

class WebSocketServer : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketServer(SmartHomeServer *parent = 0);

signals:

public slots:
    void onNewConnection();
    void processTextMessage(QString message);
    void socketDisconnected();
    void onValueChanged(QString id, QString resource, QVariantMap value);
    void onDeviceListUpdate();

private:
    SmartHomeServer* m_server;
    QWebSocketServer* m_webSocketServer;
    QList<QWebSocket*> m_socketList;
};

#endif // WEBSOCKETSERVER_H
