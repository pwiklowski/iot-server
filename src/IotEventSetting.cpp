#include "IotEventSetting.h"

IotEventSetting::IotEventSetting(QString source,
                                 quint8 source_id,
                                 quint8 source_id_value,
                                 QString destination,
                                 QString destination_variable,
                                 quint8 destination_value,
                                 QObject *parent) :
    QObject(parent),
    m_source(source),
    m_source_id(source_id),
    m_source_id_value(source_id_value),
    m_destination(destination),
    m_destination_variable(destination_variable),
    m_destination_value(destination_value)
{

}


