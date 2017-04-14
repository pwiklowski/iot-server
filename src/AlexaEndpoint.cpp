#include "AlexaEndpoint.h"
#include "QTcpSocket"
#include "QJsonObject"
#include "QJsonDocument"

#define PORT_NUMBER 12345

AlexaEndpoint::AlexaEndpoint(QObject *parent) : QObject(parent)
{
    connect(&m_server, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));
    m_server.listen(QHostAddress::Any, PORT_NUMBER);
}


void AlexaEndpoint::handleNewConnection(){
    qDebug() << "handleNewConnection";

    QTcpSocket* socket = m_server.nextPendingConnection();
    connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));

    socket->waitForReadyRead();
    QString request = socket->readAll();
    QString payload = request.split("\r\n\r\n").at(1);
    qDebug() << payload;
    QJsonObject req =  QJsonDocument::fromJson(payload.toLatin1()).object();

    QJsonObject res = handleCommand();
    QString response = QJsonDocument(res).toJson();

    socket->write("HTTP/1.1 200 OK\r\n");
    socket->write("Server: Qt Web Server\r\n");
    socket->write("Content-Type: text/html; charset=utf-8\r\n");
    QString responseLength = "Content-Length: " + QString::number(response.length()) + "\r\n";
    socket->write(responseLength.toUtf8());
    socket->write("Connection: close\r\n");
    socket->write("\r\n");
    socket->write(response.toUtf8());
    socket->disconnectFromHost();
}


QJsonObject AlexaEndpoint::handleCommand(){
    QJsonObject response;
    response["version"] = "1.0";


    QJsonObject outputSpeech;
    outputSpeech["type"] = "PlainText";
    outputSpeech["text"] = "Yey";


    QJsonObject res;
    res["shouldEndSession"] = true;
    res["outputSpeech"] = outputSpeech;


    response["response"] = res;
    return response;
}
