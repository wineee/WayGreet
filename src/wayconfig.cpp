#include "wayconfig.h"

WayConfig::WayConfig(QObject *parent)
    : QObject{ parent }
{
    m_config = new QSettings("WayGreet"); // QSettings::Format::NativeFormat);
    // "/etc/waygreet.conf"
    qDebug() << "@@@@@@@@@@@@@@@@@@@@@@@2" << m_config->fileName();

    Q_ASSERT(!m_instance);
    m_instance = this;
}

WayConfig *::WayConfig::instance()
{
    return m_instance;
}

QUrl WayConfig::background() const
{
    m_config->beginGroup("theme");
    auto path = m_config->value("background").toString();
    return QUrl::fromLocalFile(path);
}
