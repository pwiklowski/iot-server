#include "ScriptRunner.h"
#include "QDebug"


ScriptRunner::ScriptRunner(QString scriptId, QString script, QObject *parent) : QObject(parent)
{
    m_process = new QProcess();

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("NODE_PATH", "/usr/local/lib/node_modules");

    m_process->setProcessEnvironment(env);
    m_script = script;
    m_scriptId = scriptId;

    m_tempFile.open();
    m_tempFile.write(script.toLatin1());
    m_tempFile.close();
}


void ScriptRunner::start(){
    QStringList args;
    args.append(m_tempFile.fileName());
    m_process->start("node", args);
    connect(m_process, SIGNAL(finished(int)), this, SLOT(finish(int)));
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(kill()));
    m_timer.setInterval(5000);
    m_timer.start();
}
void ScriptRunner::kill(){
    m_process->kill();
    qDebug() << "ScriptRunner::kill" << m_process->readAll();
    emit finished();
}

void ScriptRunner::finish(int exitCode){
    m_timer.stop();
    emit finished();
}

ScriptRunner::~ScriptRunner(){

}
