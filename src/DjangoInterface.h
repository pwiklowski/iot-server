#ifndef DJANGOINTERFACE_H
#define DJANGOINTERFACE_H

#include <QObject>
#include "SmartHomeServer.h"
#include "QTcpServer"
#include "QTcpSocket"

#include "qhttpserver.hpp"
#include "qhttpserverresponse.hpp"
#include "qhttpserverrequest.hpp"


using namespace qhttp::server;


class DjangoInterface : public QObject
{
    Q_OBJECT
public:
    explicit DjangoInterface(SmartHomeServer* controller, QObject *parent = 0);
    void getScripts(QString id);



signals:

public slots:
    void handleRequest(QHttpRequest* req, QHttpResponse* res);
private:
    SmartHomeServer* m_controller;
    QHttpServer m_httpServer;
};

#endif // DJANGOINTERFACE_H
