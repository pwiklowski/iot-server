#include <QCoreApplication>
#include "SmartHomeServer.h"


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    SmartHomeServer server;


    return a.exec();
}
