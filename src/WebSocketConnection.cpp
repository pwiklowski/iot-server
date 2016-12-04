#include "WebSocketConnection.h"

WebSocketConnection::WebSocketConnection(QWebSocket* socket)
{
    m_socket = socket;
    m_isAuthorized = false;
}



WebSocketConnection::~WebSocketConnection(){
    m_socket->deleteLater();
}
