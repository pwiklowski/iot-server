#include "BleScanner.h"
#include "QDebug"
#include "QString"
BleScanner::BleScanner(QObject *parent) : QObject(parent)
{

    m_scanner_process.start("sudo hcidump -R ");
    QObject::connect(&m_scanner_process, SIGNAL(readyRead()), this, SLOT(readyRead()));


}


void BleScanner::readyRead()
{
    QString data = m_scanner_process.readAll();

    QList<QString> packets =  data.split('>', QString::SkipEmptyParts);
    foreach(QString p, packets)
    {
        QList<QString> bytes = p.remove('\n').trimmed().split(' ', QString::SkipEmptyParts);

        QByteArray ar;

        foreach(QString b, bytes)
        {
           ar.append(b.toInt(0,16));
        }
        quint8 a = ar.at(7);
        QByteArray payload = ar.mid(13,ar.size()-13 - 1);
        QDataStream stream(payload);

        //Hack to ignore salon
        if (a != 0xb8)
        {
            quint8 payload_len;

            stream >> payload_len;

            QByteArray source = ar.mid(7,6);

            quint8 rssi = ar.at(ar.size()-1);

            QByteArray eventData;
            QByteArray name;

            if (payload_len > 0)
            {

                while(!stream.atEnd())
                {
                    quint8 len, type;

                    stream >> len;
                    stream >> type;

                    if (type == 0xFF) //manufacturer data
                    {
                        eventData.resize(len-1);
                        stream.readRawData(eventData.data(),len-1);
                    }
                    else if (type == 0x09) //name
                    {
                        name.resize(len-1);
                        stream.readRawData(name.data(),len-1);

                    }
                    else
                    {
                        stream.skipRawData(len-1);
                    }
                }
                qDebug() << name << source.toHex() << eventData.toHex() << rssi;
                emit iotEventReceived(source.toHex(), eventData);
            }
        }
    }
}



BleScanner::~BleScanner()
{

}

