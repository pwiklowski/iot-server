#ifndef IOTEVENTSETTING_H
#define IOTEVENTSETTING_H

#include <QObject>

class IotEventSetting : public QObject
{
    Q_OBJECT
public:
    explicit IotEventSetting(QString source,
        quint8 source_id,
        quint8 source_id_value,
        QString destination,
        QString destination_variable,
        quint8 destinationValue,
        QObject *parent = 0);





    QString getSource() { return m_source;}

    quint8 getSourceId() { return m_source_id;}
    quint8 getSourceIdValue() { return m_source_id_value;}
    QString getDestination() { return m_destination;}
    QString getDestinationVariable() { return m_destination_variable;}
    quint8 getDestinationValue() { return m_destination_value;}

signals:

public slots:


private:
    QString m_source;

    quint8 m_source_id;
    quint8 m_source_id_value;
    QString m_destination;
    QString m_destination_variable;

    quint8 m_destination_value;
};
#endif // IOTEVENTSETTING_H
