#include "QUrl"
#include "SmartHomeServer.h"
#include "IotEvent.h"
#include "QThread"
#include "Settings.h"
#include "QDateTime"
#include <poll.h>
#include "QTimer"

SmartHomeServer::SmartHomeServer(QObject *parent) :
    QObject(parent)
{
     m_parent = parent;
    Settings* settings = new Settings(this);
    m_server.listen(QHostAddress::Any, 9999);

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setConnectOptions("QSQLITE_OPEN_READONLY");
    //m_db.setDatabaseName(settings->getValue("database").toString());
    m_db.setDatabaseName("/home/pawwik/dev/iot_webui/db.sqlite3");

    if (!m_db.open())
    {
        qWarning() << "Unable to open database";
    }

    temp = engine.newObject();


    m_client = new OICClient([&](COAPPacket* packet){
        this->send_packet(packet);
    });
    m_client->start("","");

    pthread_create(&m_thread, NULL, &SmartHomeServer::run, this);
    findDevices();

    QTimer* t = new QTimer(this);
    t->setInterval(5000);
    t->setSingleShot(false);


    qDebug() << getScripts("0685B960-736F-46F7-BEC0-9E6CBD61ADC1");
}


QStringList SmartHomeServer::getScripts(QString id)
{
    QStringList scripts;

    QSqlQuery query = m_db.exec("SELECT script FROM webui_iotscripts WHERE di='"+id+"';");
    qDebug() << query.lastError().text();
    while (query.next()) {
        scripts << query.value(0).toString();
    }
    return scripts;
}
void SmartHomeServer::saveGlobalObject(QString key, QScriptValue obj)
{
    m_cloudScriptStorage.insert(key, obj);
}
QScriptValue SmartHomeServer::getGlobalObject(QString key)
{
    if (!m_cloudScriptStorage.keys().contains(key))
        return engine.newObject();
    return m_cloudScriptStorage.value(key);
}






void SmartHomeServer::iotEventReceived(QString source,  QByteArray eventData)
{

}

QList<Device *> SmartHomeServer::getClientList()
{
    return m_clientList;
}
Device *SmartHomeServer::getClient(QString id)
{
    foreach(Device* client, m_clientList)
    {
        if (client->getID() == id)
        {
            return client;
        }
    }
    return 0;
}

bool SmartHomeServer::setValue(QString id, QString resource, QVariantMap value)
{
    Device* client = getClient(id);

    if (client!=0)
    {
        DeviceVariable* variable = client->getVariable(resource);

        if (variable != 0){
            variable->post(value);

        }


        return true;
    }
    return false;
}
bool SmartHomeServer::isDeviceOnList(QString id){
   for (Device* d: m_clientList){
       if (d->getID() == id){
           return true;
       }
   }
   return false;
}

void SmartHomeServer::findDevices()
{
    m_client->searchDevices([&](COAPPacket* packet){
        if (!packet) return false;
        cbor message;
        cbor::parse(&message, packet->getPayload());

        if (packet->getCode() != COAP_RSPCODE_CONTENT)
            return false;

        for (uint16_t i=0; i<message.toArray()->size(); i++){
            cbor device = message.toArray()->at(i);

            cbor naame = device.getMapValue("n");
            String name = device.getMapValue("n").toString();
            String di= device.getMapValue("di").toString();

            qDebug() << "new device"<<  name.c_str() << di.c_str();

            if (isDeviceOnList(QString(di.c_str()))) continue;

            cbor links = device.getMapValue("links");
            OICDevice* dev = new OICDevice(di, name, packet->getAddress(), m_client);

            for (uint16_t j=0; j< links.toArray()->size(); j++){
                cbor link = links.toArray()->at(j);


                String href = link.getMapValue("href").toString();
                String rt = link.getMapValue("rt").toString();
                String iff = link.getMapValue("if").toString();

                dev->getResources()->push_back(new OICDeviceResource(href, iff, rt, dev, m_client));
            }

                m_variablesStorage.insert(di.c_str(), new QVariantMap());
                Device* d = new Device(dev, this);
                connect(d, SIGNAL(variablesChanged(QString,QVariantMap)), this, SLOT(onValueChanged(QString,QVariantMap)));
                m_clientList.append(d);
        }
        emit devicesChanged();
    });
}
QScriptValue logger( QScriptContext * ctx, QScriptEngine * eng ) {
    return QScriptValue();
}
void SmartHomeServer::onValueChanged(QString resource, QVariantMap value){
    Device* d = (Device*)sender();

    qDebug() << "onValueChanged" << d->getID() << resource << value;

    QVariantMap* v = m_variablesStorage.value(d->getID());
    v->insert(resource, value);


    QVariantMap eventInfo;

    QScriptValue event = engine.newObject();
    event.setProperty("source",d->getID());
    event.setProperty("resource", resource);
    event.setProperty("data", engine.toScriptValue(value));

    engine.globalObject().setProperty("event", event);
    engine.globalObject().setProperty("server", engine.newQObject(this));

    QScriptValue time = engine.newObject();

    time.setProperty("minute", QDateTime::currentDateTime().time().minute());
    time.setProperty("hour", QDateTime::currentDateTime().time().hour());
    time.setProperty("second", QDateTime::currentDateTime().time().second());
    time.setProperty("day", QDateTime::currentDateTime().date().day());
    time.setProperty("dayOfWeek", QDateTime::currentDateTime().date().dayOfWeek());
    time.setProperty("month", QDateTime::currentDateTime().date().month());

    engine.globalObject().setProperty("time", time);

    QStringList scripts  = getScripts(d->getID());
    qDebug() << "Found " << scripts.size() << "scripts";
    foreach(QString script, scripts)
    {
        QScriptValue error = engine.evaluate(QUrl::fromPercentEncoding(script.toLatin1()));
        qDebug() << "error" << error.toString();
    }
}


String SmartHomeServer::convertAddress(sockaddr_in a){
    char addr[30];
    sprintf(addr, "%d.%d.%d.%d %d",
            (uint8_t) (a.sin_addr.s_addr),
            (uint8_t) (a.sin_addr.s_addr >> 8),
            (uint8_t) (a.sin_addr.s_addr >> 16 ),
            (uint8_t) (a.sin_addr.s_addr >> 24),
            htons(a.sin_port));

    return addr;
}
void* SmartHomeServer::run(void* param){
    SmartHomeServer* d = (SmartHomeServer*) param;
    OICClient* oic_server = d->getClient();
    COAPServer* coap_server = oic_server->getCoapServer();


    const int on = 1;
    int fd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    d->setSocketFd(fd);

    struct sockaddr_in serv,client;
    struct ip_mreq mreq;

    serv.sin_family = AF_INET;
    serv.sin_port = 0;
    serv.sin_addr.s_addr = htonl(INADDR_ANY);

    uint8_t buffer[1024];
    socklen_t l = sizeof(client);
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
        qDebug("Unable to set reuse");
        return 0;
    }
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000000;

    if( bind(fd, (struct sockaddr*)&serv, sizeof(serv) ) == -1)
    {
        qDebug("Unable to bind");
        return 0;
    }

    struct pollfd pfd;
    int res;

    pfd.fd = fd;
    pfd.events = POLLIN;

    size_t rc;
    while(1){
        rc = poll(&pfd, 1, 200); // 1000 ms timeout
        if (rc >0){
            rc = recvfrom(fd,buffer,sizeof(buffer),0,(struct sockaddr *)&client,&l);
            COAPPacket* p = COAPPacket::parse(buffer, rc, d->convertAddress(client).c_str());
            coap_server->handleMessage(p);
        }
        coap_server->tick();
    }
}
void SmartHomeServer::send_packet(COAPPacket* packet){
    String destination = packet->getAddress();
    size_t pos = destination.find(" ");
    String ip = destination.substr(0, pos);
    uint16_t port = atoi(destination.substr(pos).c_str());

    struct sockaddr_in client;

    client.sin_family = AF_INET;
    client.sin_port = htons(port);
    client.sin_addr.s_addr = inet_addr(ip.c_str());

    qDebug() << "Send packet mid=" << packet->getMessageId() << "destination=" <<destination.c_str();
    send_packet(client, packet);
}
void SmartHomeServer::send_packet(sockaddr_in destination, COAPPacket* packet){

    uint8_t buffer[1024];
    size_t response_len;
    socklen_t l = sizeof(destination);
    packet->build(buffer, &response_len);

    sendto(m_socketFd, buffer, response_len, 0, (struct sockaddr*)&destination, l);
}
