#include "QDebug"
#include "Settings.h"
#include "QJsonDocument"
#include "QJsonObject"
#include "QFile"
#include "QCoreApplication"

Settings::Settings(QObject *parent) :
    QObject(parent)
{
    QFile settingsFile("/usr/local/wiklosoft/ioserver.config");
    if (settingsFile.open(QFile::ReadOnly))
    {
        QString set = settingsFile.readAll();
        QJsonDocument d = QJsonDocument::fromJson(set.toUtf8());
        QJsonObject root = d.object();
        qDebug() << d.toJson();
        m_settings = root.toVariantMap();
        settingsFile.close();
    }
}
QVariant Settings::getValue(QString key)
{
    return m_settings.value(key);
}
