#ifndef WEBSOCKETCONNECTION_H
#define WEBSOCKETCONNECTION_H

#include <QObject>
#include "QWebSocket"


class WebSocketConnection
{
public:
    WebSocketConnection(QWebSocket* socket);
    ~WebSocketConnection();

    QWebSocket* getSocket(){return m_socket; }

    QStringList* getDeviceSubscription() { return &m_deviceSubscriptions; }
    QStringList* getScriptSubscription() { return &m_scriptsSubscriptions; }

private:
    QWebSocket* m_socket;

    QStringList m_scriptsSubscriptions;
    QStringList m_deviceSubscriptions;
};

#endif // WEBSOCKETCONNECTION_H
