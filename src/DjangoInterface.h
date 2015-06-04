#ifndef DJANGOINTERFACE_H
#define DJANGOINTERFACE_H

#include <QObject>
#include "SmartHomeServer.h"
#include "QTcpServer"



typedef enum
{
    DJANGO_GET_DEVICE_LIST,
    DJANGO_DEVICE_LIST,

    DJANGO_GET_BUTTON_LIST,
    DJANGO_BUTTON_LIST,

    DJANGO_GET_VALUES_LIST,
    DJANGO_VALUES_LIST,

    DJANGO_GET_SENSORS_LIST,
    DJANGO_SENSORS_LIST,

    DJANGO_GET_DEVICE_DESCRIPTION,
    DJANGO_DEVICE_DESCRIPTION,


}SmartHomeMessageTypes;

class DjangoInterface : public QObject
{
    Q_OBJECT
public:
    explicit DjangoInterface(SmartHomeServer* controller, QObject *parent = 0);

signals:

public slots:

    void handleNewConnection();
    void removeClient();


    void readData();
    void parseMessages(quint8 type, QByteArray payload, QTcpSocket* socket);

private:
    SmartHomeServer* m_controller;
    QTcpServer m_server;


};

#endif // DJANGOINTERFACE_H
