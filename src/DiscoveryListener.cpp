#include "DiscoveryListener.h"
#include "QNetworkInterface"

DiscoveryListener::DiscoveryListener(QObject *parent) :
    QObject(parent)
{
    m_socket = new QUdpSocket(this);


    m_socket->bind(QHostAddress("239.255.255.250"),1901);
    m_socket->joinMulticastGroup(QHostAddress("239.255.255.250"));
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(readDatagram()));
}

void DiscoveryListener::readDatagram()
{
    while(m_socket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(m_socket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        m_socket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        QString senderSubnet = sender.toString().mid(0, sender.toString().lastIndexOf("."));
        QString serverAddress;
        foreach (const QHostAddress &address, QNetworkInterface::allAddresses())
        {
            if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
            {
                QString interfacesubnet = address.toString().mid(0, address.toString().lastIndexOf("."));

                if (interfacesubnet == senderSubnet)
                {
                   serverAddress = address.toString();
                   break;
                }
            }
        }
        QString response = QString("ALIVE %1:9999").arg(serverAddress);
        qDebug() << "send response" << response;
        m_socket->writeDatagram(response.toLatin1(), sender, senderPort);


        qDebug() <<"DiscoveryListenery"<< datagram;
    }


}
