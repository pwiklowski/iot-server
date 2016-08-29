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

    if (req->url().toString() == "/devices"){
        QList<IotDevice*> devices = m_controller->getClientList();
        QJsonObject root;
        QString json;
        QJsonArray devs;

        for(int i=0; i<devices.length();i++)
        {
            IotDevice* device = devices.at(i);
            QJsonObject dev;
            dev["name"] = device->getName();
            dev["id"] = device->getID().remove("device:");

            QJsonArray vars;
            for(int i=0; i<device->getVariables()->size(); i++){
                IotDeviceVariable* var = device->getVariables()->at(i);
                vars.append(QJsonValue::fromVariant(var->getResource()));
            }
            dev["variables"] = vars;

            devs.append(dev);
        }
        root.insert("devices", devs);
        json = QJsonDocument(root).toJson(QJsonDocument::Compact);

        res->setStatusCode(qhttp::ESTATUS_OK);
        res->end(json.toLatin1());
    }else if (req->url().toString().startsWith("/values")){
        QString id = req->url().toString().split("/",QString::SkipEmptyParts).at(1);

        QString json;

        IotDevice* dev = m_controller->getDeviceById(id);

        QVariantMap* storedVariables = m_controller->getVariablesStorage(id);

        QJsonArray vars;
        if (dev)
        {
            for(int i=0; i<dev->getVariables()->size(); i++)
            {
                IotDeviceVariable* var = dev->getVariables()->at(i);
                QVariantMap res = storedVariables->value(var->getResource()).toMap();

                QJsonObject v;
                v["name"] = var->getResource();
                v["values"]= QJsonObject::fromVariantMap(res);
                vars.append(v);
            }
            json = QJsonDocument(vars).toJson();


            res->setStatusCode(qhttp::ESTATUS_OK);
            res->end(json.toLatin1());
        }else{
            res->setStatusCode(qhttp::ESTATUS_NOT_FOUND);
            res->end("404");
        }
    }else if (devicesScripts.indexIn(url) == 0){
        QString id = devicesScripts.cap(1);

        if (req->method() == qhttp::EHTTP_GET){


        }else if (req->method() == qhttp::EHTTP_POST){


        }else if (req->method() == qhttp::EHTTP_DELETE){


        }


        res->setStatusCode(qhttp::ESTATUS_OK);
        res->end("OK");
    }else if (runScript.indexIn(url) == 0){
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
