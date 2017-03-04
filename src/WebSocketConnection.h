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

    void setAuthorized(bool auth) { m_isAuthorized = auth; }

    bool isAuthorized() { return m_isAuthorized; }

private:
    QWebSocket* m_socket;

    QStringList m_scriptsSubscriptions;
    QStringList m_deviceSubscriptions;

    bool m_isAuthorized;
};

#endif // WEBSOCKETCONNECTION_H
