#include "ScriptRunner.h"
#include "QDebug"


ScriptRunner::ScriptRunner(QString scriptId, QString script, QObject *parent) : QObject(parent)
{
    m_process = new QProcess();
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
}

void ScriptRunner::finish(int exitCode){
    emit finished();
}

ScriptRunner::~ScriptRunner(){

}
