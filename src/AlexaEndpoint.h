#ifndef ALEXAENDPOINT_H
#define ALEXAENDPOINT_H

#include <QObject>
#include "QTcpServer"

class AlexaEndpoint : public QObject
{
    Q_OBJECT
public:
    explicit AlexaEndpoint(QObject *parent = 0);

    QJsonObject handleCommand();
signals:

public slots:
    void handleNewConnection();
private:
    QTcpServer m_server;

};

#endif // ALEXAENDPOINT_H
