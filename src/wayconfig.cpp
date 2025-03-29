#include "wayconfig.h"

WayConfig::WayConfig(QObject *parent)
    : QObject{ parent }
{
    m_config = new QSettings("WayGreet", "", this); // QSettings::Format::NativeFormat);
    // "/etc/waygreet.conf"
    qDebug() << "@@@@@@@@@@@@@@@@@@@@@@@2" << m_config->fileName();

    Q_ASSERT(!m_instance);
    m_instance = this;
}

WayConfig * ::WayConfig::instance()
{
    return m_instance;
}

QUrl WayConfig::background() const
{
    m_config->beginGroup("Theme");
    auto path = m_config->value("background").toString();
    m_config->endGroup();
    return QUrl::fromLocalFile(path);
}

bool WayConfig::showX11Session() const
{
    return m_config->value("showX11Session", false).toBool();
}

QStringList WayConfig::waylandSessionDir() const
{
    QStringList sessionDir;
    if (auto path = m_config->value("waylandSessionDir").toString(); !path.isEmpty())
        sessionDir << path;
    sessionDir << "/usr/local/share/wayland-sessions"
               << "/usr/share/wayland-sessions";
    return sessionDir;
}

QStringList WayConfig::x11SessionDir() const
{
    QStringList sessionDir;
    if (auto path = m_config->value("x11SessionDir").toString(); !path.isEmpty())
        sessionDir << path;
    sessionDir << "/usr/local/share/xsessions"
               << "/usr/share/xsessions";
    return sessionDir;
}

QString WayConfig::lastSession() const
{
    return m_config->value("lastSession").toString();
}

void WayConfig::setLastSession(const QString &session)
{
    m_config->setValue("lastSession", session);
}
