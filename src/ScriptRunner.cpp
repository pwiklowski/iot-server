#include "ScriptRunner.h"
#include "QDebug"
#include "QJsonDocument"
#include "QJsonObject"


ScriptRunner::ScriptRunner(WebSocketServer* server, QString scriptId, QString script, QObject *parent) : QObject(parent)
{
    m_process = new QProcess();
    m_socketServer = server;

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("NODE_PATH", "/usr/local/lib/node_modules");

    m_process->setProcessEnvironment(env);
    m_script = script;
    m_scriptId = scriptId;

    m_tempFile.open();
    m_tempFile.write(script.toLatin1());
    m_tempFile.close();
}


void ScriptRunner::start(QVariantMap event){
    QString eventJson = QJsonDocument::fromVariant(event).toJson();
    m_socketServer->onLogMessage(m_scriptId, "Start with arg: " + eventJson);

    QStringList args;
    args.append(m_tempFile.fileName());
    args.append(eventJson);
    connect(m_process, SIGNAL(finished(int)), this, SLOT(finish(int)));
    connect(m_process, SIGNAL(readyRead()), this, SLOT(onLog()));
    connect(m_process, SIGNAL(readyReadStandardError()), this, SLOT(onError()));
    m_process->start("node", args);
    m_runTimer.start();

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(kill()));
    m_timer.setInterval(5000);
    m_timer.start();
}
void ScriptRunner::kill(){
    m_process->kill();
    m_socketServer->onLogMessage(m_scriptId, "Timeout. Kill script");
    emit finished();
}

void ScriptRunner::onError(){
    QString line = "Error: " + m_process->readAllStandardError();

    QStringList lines = line.split('\n');
    foreach(QString l, lines)
        m_socketServer->onLogMessage(m_scriptId, l);
}

void ScriptRunner::onLog(){
    QString line = m_process->readLine();
    m_socketServer->onLogMessage(m_scriptId, line);
}

void ScriptRunner::finish(int){
    m_socketServer->onLogMessage(m_scriptId, "Finished. Executed in " + QString::number(m_runTimer.elapsed()) + "ms.");

    m_timer.stop();
    emit finished();
}

ScriptRunner::~ScriptRunner(){

}
