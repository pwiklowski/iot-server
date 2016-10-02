#include "DjangoInterface.h"
#include "QJsonDocument"
#include "QJsonObject"
#include "QJsonArray"
#include "QDataStream"


DjangoInterface::DjangoInterface(SmartHomeServer* controller, QObject *parent) :
    QObject(parent),
    m_controller(controller),
    m_httpServer(controller)
{
    m_httpServer.listen(QHostAddress::Any, 7000);
    if (!m_httpServer.isListening() ) {
        qDebug("failed to listen");
    }

    connect(&m_httpServer, SIGNAL(newRequest(QHttpRequest*,QHttpResponse*)), SLOT(handleRequest(QHttpRequest*,QHttpResponse*)));
}

void DjangoInterface::handleRequest(QHttpRequest* req, QHttpResponse* res){
    qDebug() << req->url();
    QString url = req->url().toString();

    QRegExp devicesScripts("/device/(.+)/script");
    QRegExp runScript("/script/(.+)/run");


    qDebug() << devicesScripts.indexIn(req->url().toString());
    qDebug() << devicesScripts.exactMatch(req->url().toString());

  if (runScript.indexIn(url) == 0){
        QString id = runScript.cap(1);


        req->collectData();
        req->onEnd([=]() {
            qDebug() << QString(req->collectedData());

            QJsonDocument json =  QJsonDocument::fromJson(req->collectedData());
            QScriptEngine* engine = m_controller->getEngine();

            QScriptValue e = engine->newObject();

            foreach(QString key, json.object().keys()){
                e.setProperty(key, engine->newVariant(json.object().value(key).toVariant()));
            }

            m_controller->runScript(id, e);
            qDebug() << "Run script" << id;
        });

    }else{
        res->setStatusCode(qhttp::ESTATUS_NOT_FOUND);
        res->end("404");
    }


    return;
}
