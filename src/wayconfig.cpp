// Copyright (C) 2025 rewine <luhongxu@deepin.org>.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wayconfig.h"

WayConfig::WayConfig(QObject *parent)
    : QObject{ parent }
{
    m_config = new QSettings("dwapp", "waygreet", this);
    // $HOME/.config/dwapp/waygreet.conf
    // for each directory <dir> in $XDG_CONFIG_DIRS: <dir>/dwapp/waygreet.conf
    qDebug() << "Load config in: " << m_config->fileName();

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

QString WayConfig::cursorTheme() const
{
    return m_config->value("cursorTheme", "default").toString();
}

QSize WayConfig::cursorSize() const
{
    return m_config->value("cursorSize", QSize(24, 24)).toSize();
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

QString WayConfig::lastUser() const
{
    return m_config->value("lastUser").toString();
}

void WayConfig::setLastUser(const QString &name)
{
    m_config->setValue("lastUser", name);
}

int WayConfig::minimumUid() const
{
    // TODO: read from /etc/login.defs
    return 1000;
}

int WayConfig::maximumUid() const
{
    return 29999;
}

QStringList WayConfig::hideUsers() const
{
    return m_config->value("hideUsers").toString().split(";");
}

QString WayConfig::powerOffCommand() const
{
    return QStringLiteral("/usr/bin/systemctl poweroff");
}

QString WayConfig::rebootCommand() const
{
    return QStringLiteral("/usr/bin/systemctl reboot");
}
