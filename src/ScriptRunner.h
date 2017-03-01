#ifndef SCRIPTRUNNER_H
#define SCRIPTRUNNER_H

#include <QObject>
#include "QProcess"
#include "QTemporaryFile"
#include "QTimer"
#include "QElapsedTimer"
#include "WebSocketServer.h"

class ScriptRunner : public QObject
{
    Q_OBJECT
public:
    explicit ScriptRunner(WebSocketServer* server, QString scriptId, QString script, QObject *parent = 0);
    ~ScriptRunner();

signals:
    void finished();

public slots:
    void finish(int exitCode);
    void start(QVariantMap event);
    void kill ();
    void onLog();
    void onError();

private:
    QProcess* m_process;
    QTemporaryFile m_tempFile;
    QString m_script;
    QString m_scriptId;
    QTimer m_timer;
    QElapsedTimer m_runTimer;
    WebSocketServer* m_socketServer;
};

#endif // SCRIPTRUNNER_H
