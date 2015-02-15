#ifndef SMARTHOMECLIENT_H
#define SMARTHOMECLIENT_H

#include <QObject>
#include "QTcpSocket"
#include "QTimer"


#define SMART_HOME_HEADER 0xFA34B6DE
class SmartHomeServer;

class SmartHomeClient : public QObject
{
    Q_OBJECT

    typedef enum
    {
        GET_DEVICE_DESCRIPTION,
        DEVICE_DESCRIPTION,

        GET_DEVICE_LIST,
        DEVICE_LIST,
        DEVICE_LIST_CHANGED,

        CHANGE_VALUE,
        VALUE_CHANGED,

        GET_VALUES_LIST,
        VALUES_LIST,

        PING,
        PING_RESPONSE

    }SmartHomeMessageTypes;



public:
    explicit SmartHomeClient(QTcpSocket* socket, SmartHomeServer *parent = 0);

    QString getName(){return m_name;}
    QString getID(){return m_id;}
    QString getDescription(){return m_description;}
signals:
    void disconnected();
public slots:
    void readData();
    void parseMessages(quint8 type, QByteArray payload);

    void getDeviceDescription();
    void sendDeviceList();
    void sendChangeValue(QByteArray payload);
    void deviceListChanged();
    void sendDeviceListChanged();
    void sendVariableChanged(QByteArray payload);
    void sendGetValuesList(QByteArray payload);
    void sendValuesList(QByteArray payload);
    void sendPing();
private:
    QString generateDeviceList();

    SmartHomeServer* m_server;
    QTcpSocket* m_socket;
    QByteArray m_buffer;
    QString m_description;
    QString m_name;
    QString m_id;

    QTimer m_ping_timer;
    QTimer m_remove_timer;

};

#endif // SMARTHOMECLIENT_H
