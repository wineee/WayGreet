// Copyright (C) 2013 Abdurrahman AVCI <abdurrahmanavci@gmail.com>
// Copyright (C) 2025 rewine <luhongxu@deepin.org>
// SPDX-License-Identifier: GPL-2.0-or-later and GPL-3.0-or-later

// This file is modified from `sddm`

#pragma once

#include <QObject>
#include <QVector>
#include <QQmlEngine>

class PowerManagerBackend;

class PowerManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit PowerManager(QObject *parent = 0);
    ~PowerManager();

    enum Capability {
        None = 0x0000,
        PowerOff = 0x0001,
        Reboot = 0x0002,
        Suspend = 0x0004,
        Hibernate = 0x0008,
        HybridSleep = 0x0010
    };
    Q_ENUM(Capability);
    Q_DECLARE_FLAGS(Capabilities, Capability)

public Q_SLOTS:
    Capabilities capabilities() const;

    void powerOff() const;
    void reboot() const;
    void suspend() const;
    void hibernate() const;
    void hybridSleep() const;

    inline bool canPowerOff() const { return capabilities().testFlag(Capability::PowerOff); };
    inline bool canReboot() const { return capabilities().testFlag(Capability::Reboot); };
    inline bool canSuspend() const { return capabilities().testFlag(Capability::Suspend); };
    inline bool canHibernate() const { return capabilities().testFlag(Capability::Hibernate); };
    inline bool canHybridSleep() const { return capabilities().testFlag(Capability::HybridSleep); };

private:
    QVector<PowerManagerBackend *> m_backends;
};
