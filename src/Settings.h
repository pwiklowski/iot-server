#ifndef SETTINGS_H
#define SETTINGS_H
#include "QVariantMap"
#include <QObject>

class Settings : public QObject
{
    Q_OBJECT
public:
    explicit Settings(QObject *parent = 0);
    QVariant getValue(QString key);

signals:

public slots:
private:
    QVariantMap m_settings;
};

#endif // SETTINGS_H
