// Copyright (C) 2025 rewine <luhongxu@deepin.org>.
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QSettings>
#include <QSize>

class WayConfig : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QUrl background READ background CONSTANT)
    Q_PROPERTY(QString cursorTheme READ cursorTheme CONSTANT)
    Q_PROPERTY(QSize cursorSize READ cursorSize CONSTANT)

public:
    explicit WayConfig(QObject *parent = nullptr);
    static WayConfig *instance();

    QUrl background() const;

    QString cursorTheme() const;
    QSize cursorSize() const;

    bool showX11Session() const;
    QStringList waylandSessionDir() const;
    QStringList x11SessionDir() const;

    QString lastSession() const;
    void setLastSession(const QString &session);

    QString lastUser() const;
    void setLastUser(const QString &name);

    int minimumUid() const;
    int maximumUid() const;
    QStringList hideUsers() const;

    QString powerOffCommand() const;
    QString rebootCommand() const;

private:
    QSettings *m_config;

    inline static WayConfig *m_instance = nullptr;
};
