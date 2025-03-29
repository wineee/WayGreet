/***************************************************************************
 * Copyright (c) 2013 Abdurrahman AVCI <abdurrahmanavci@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 ***************************************************************************/
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
    Q_ENUMS(Capability);
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
