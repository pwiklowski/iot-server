#include "IotDevice.h"

IotDevice::IotDevice(QObject *parent) : QObject(parent)
{

}

IotDeviceVariable::IotDeviceVariable(QObject* parent): QObject(parent)
{

}

IotDeviceVariable* IotDevice::getVariable(QString resource) {
    for(int i=0; i<m_variables.length();i++){
        if(m_variables.at(i)->getResource() == resource) return m_variables.at(i);
    }

    return 0;
}

