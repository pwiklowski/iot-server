#ifndef SERIALBLESCANNER_H
#define SERIALBLESCANNER_H

#include "QtSerialPort/QSerialPort"
#include "QMutex"


class SerialBleScanner : public QObject
{
    Q_OBJECT
public:
    SerialBleScanner(QObject *parent=0);

signals:
     void iotEventReceived(QString address, QByteArray eventData);

public slots:


    void read();
    void parseMessage();
private:
    QMutex mutex;

    QSerialPort m_serial;
    QByteArray m_buffer;
};

#endif // SERIALBLESCANNER_H
