#ifndef RFM69DEVICECONTROLLER_H
#define RFM69DEVICECONTROLLER_H

#include <QObject>
#include "rfm69.h"

class Rfm69DeviceController: public QObject
{
    Q_OBJECT
public:
    Rfm69DeviceController();


    void start();
    static void* run(void* param);
signals:

public slots:


private:
    pthread_t m_thread;
};

#endif // RFM69DEVICECONTROLLER_H
