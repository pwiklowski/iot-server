#ifndef BLESCANNER_H
#define BLESCANNER_H

#include <QObject>
#include "QProcess"

class BleScanner : public QObject
{
    Q_OBJECT
public:
    explicit BleScanner(QObject *parent = 0);
    ~BleScanner();

signals:
    void iotEventReceived(QString source,  QByteArray eventData);
public slots:
    void readyRead();
private:
    QProcess m_scanner_process;


};

#endif // BLESCANNER_H
