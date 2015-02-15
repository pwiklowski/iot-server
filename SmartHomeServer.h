#ifndef SMARTHOMESERVER_H
#define SMARTHOMESERVER_H

#include <QObject>
#include "QTcpServer"
#include "SmartHomeClient.h"

class SmartHomeServer : public QObject
{
    Q_OBJECT
public:
    explicit SmartHomeServer(QObject *parent = 0);

signals:
    void clientListChanged();
public slots:
    void handleNewConnection();
    QList<SmartHomeClient*> getClientList();
    void removeClient();
private:
    QString generateHash();

    QTcpServer m_server;
    QList<SmartHomeClient*> m_clientList;


};

#endif // SMARTHOMESERVER_H
