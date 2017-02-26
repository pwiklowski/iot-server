#ifndef SCRIPTRUNNER_H
#define SCRIPTRUNNER_H

#include <QObject>
#include "QProcess"
#include "QTemporaryFile"

class ScriptRunner : public QObject
{
    Q_OBJECT
public:
    explicit ScriptRunner(QString scriptId, QString script, QObject *parent = 0);
    ~ScriptRunner();

signals:
    void finished();

public slots:
    void finish(int exitCode);
    void start();

private:
    QProcess* m_process;
    QTemporaryFile m_tempFile;
    QString m_script;
    QString m_scriptId;
};

#endif // SCRIPTRUNNER_H
