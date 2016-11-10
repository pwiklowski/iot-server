#include "WebSocketConnection.h"

WebSocketConnection::WebSocketConnection(QWebSocket* socket)
{
    m_socket = socket;
}



WebSocketConnection::~WebSocketConnection(){
    m_socket->deleteLater();
}
