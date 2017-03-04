#ifndef IOTEVENT_H
#define IOTEVENT_H

#include <QObject>
#include "QDebug"

class IotEvent : public QObject
{
    Q_OBJECT
public:
    explicit IotEvent(QObject *parent = 0);

signals:

public slots:
    void test() {qDebug() << "event";}

private:
    QString m_name;
    QString m_source;

};

#endif // IOTEVENT_H
