#ifndef WAYCONFIG_H
#define WAYCONFIG_H

#include <QObject>
#include <QQmlEngine>
#include <QSettings>

class WayConfig : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QUrl background READ background CONSTANT)

public:
    explicit WayConfig(QObject *parent = nullptr);
    static WayConfig *instance();

    QUrl background() const;

    bool showX11Session() const;
    QStringList waylandSessionDir() const;
    QStringList x11SessionDir() const;

    QString lastSession() const;
    void setLastSession(const QString &session);

private:
    QSettings *m_config;

    inline static WayConfig *m_instance = nullptr;
};

#endif // WAYCONFIG_H
