// Copyright (C) 2013 Abdurrahman AVCI <abdurrahmanavci@gmail.com>
// Copyright (C) 2025 rewine <luhongxu@deepin.org>
// SPDX-License-Identifier: GPL-2.0-or-later and GPL-3.0-or-later

#include "powermanager.h"
#include "helper.h"

#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusReply>
#include <QProcess>

/************************************************/
/* POWER MANAGER BACKEND                        */
/************************************************/
class PowerManagerBackend
{
public:
    PowerManagerBackend() { }

    virtual ~PowerManagerBackend() { }

    virtual PowerManager::Capabilities capabilities() const = 0;

    virtual void powerOff() const = 0;
    virtual void reboot() const = 0;
    virtual void suspend() const = 0;
    virtual void hibernate() const = 0;
    virtual void hybridSleep() const = 0;
};

/**********************************************/
/* UPOWER BACKEND                             */
/**********************************************/

const QString UPOWER_PATH = QStringLiteral("/org/freedesktop/UPower");
const QString UPOWER_SERVICE = QStringLiteral("org.freedesktop.UPower");
const QString UPOWER_OBJECT = QStringLiteral("org.freedesktop.UPower");

class UPowerBackend : public PowerManagerBackend
{
public:
    UPowerBackend(const QString &service, const QString &path, const QString &interface)
    {
        m_interface = new QDBusInterface(service, path, interface, QDBusConnection::systemBus());
    }

    ~UPowerBackend() { delete m_interface; }

    using Capabilities = PowerManager::Capabilities;

    PowerManager::Capabilities capabilities() const override
    {
        Capabilities caps(PowerManager::Capability::PowerOff | PowerManager::Capability::Reboot);

        QDBusReply<bool> reply;

        // suspend
        reply = m_interface->call(QStringLiteral("SuspendAllowed"));
        if (reply.isValid() && reply.value())
            caps |= PowerManager::Capability::Suspend;

        // hibernate
        reply = m_interface->call(QStringLiteral("HibernateAllowed"));
        if (reply.isValid() && reply.value())
            caps |= PowerManager::Capability::Hibernate;

        // return capabilities
        return caps;
    }

    void powerOff() const override
    {
        // TODO(rewine): use config
        auto command = QProcess::splitCommand(QString("/usr/bin/systemctl poweroff"));
        const QString program = command.takeFirst();
        QProcess::execute(program, command);
    }

    void reboot() const override
    {
        // TODO(rewine): use config
        auto command = QProcess::splitCommand(QString("/usr/bin/systemctl reboot"));
        const QString program = command.takeFirst();
        QProcess::execute(program, command);
    }

    void suspend() const override { m_interface->call(QStringLiteral("Suspend")); }

    void hibernate() const override { m_interface->call(QStringLiteral("Hibernate")); }

    void hybridSleep() const override { }

private:
    QDBusInterface *m_interface{ nullptr };
};

/**********************************************/
/* LOGIN1 && ConsoleKit2 BACKEND              */
/**********************************************/

const QString LOGIN1_SERVICE = QStringLiteral("org.freedesktop.login1");
const QString LOGIN1_PATH = QStringLiteral("/org/freedesktop/login1");
const QString LOGIN1_OBJECT = QStringLiteral("org.freedesktop.login1.Manager");

const QString CK2_SERVICE = QStringLiteral("org.freedesktop.ConsoleKit");
const QString CK2_PATH = QStringLiteral("/org/freedesktop/ConsoleKit/Manager");
const QString CK2_OBJECT = QStringLiteral("org.freedesktop.ConsoleKit.Manager");

class SeatManagerBackend : public PowerManagerBackend
{
public:
    SeatManagerBackend(const QString &service, const QString &path, const QString &interface)
    {
        m_interface = new QDBusInterface(service, path, interface, QDBusConnection::systemBus());
    }

    ~SeatManagerBackend() { delete m_interface; }

    PowerManager::Capabilities capabilities() const override
    {
        PowerManager::Capabilities caps = PowerManager::Capability::None;

        QDBusReply<QString> reply;

        // power off
        reply = m_interface->call(QStringLiteral("CanPowerOff"));
        if (reply.isValid() && (reply.value() == QLatin1String("yes")))
            caps |= PowerManager::Capability::PowerOff;

        // reboot
        reply = m_interface->call(QStringLiteral("CanReboot"));
        if (reply.isValid() && (reply.value() == QLatin1String("yes")))
            caps |= PowerManager::Capability::Reboot;

        // suspend
        reply = m_interface->call(QStringLiteral("CanSuspend"));
        if (reply.isValid() && (reply.value() == QLatin1String("yes")))
            caps |= PowerManager::Capability::Suspend;

        // hibernate
        reply = m_interface->call(QStringLiteral("CanHibernate"));
        if (reply.isValid() && (reply.value() == QLatin1String("yes")))
            caps |= PowerManager::Capability::Hibernate;

        // hybrid sleep
        reply = m_interface->call(QStringLiteral("CanHybridSleep"));
        if (reply.isValid() && (reply.value() == QLatin1String("yes")))
            caps |= PowerManager::Capability::HybridSleep;

        // return capabilities
        return caps;
    }

    void powerOff() const override { m_interface->call(QStringLiteral("PowerOff"), true); }

    void reboot() const override
    {
        // if (!daemonApp->testing())
        m_interface->call(QStringLiteral("Reboot"), true);
    }

    void suspend() const override { m_interface->call(QStringLiteral("Suspend"), true); }

    void hibernate() const override { m_interface->call(QStringLiteral("Hibernate"), true); }

    void hybridSleep() const override { m_interface->call(QStringLiteral("HybridSleep"), true); }

private:
    QDBusInterface *m_interface{ nullptr };
};

/**********************************************/
/* POWER MANAGER                              */
/**********************************************/
PowerManager::PowerManager(QObject *parent)
    : QObject(parent)
{
    QDBusConnectionInterface *interface = QDBusConnection::systemBus().interface();

    // check if login1 interface exists
    if (interface->isServiceRegistered(LOGIN1_SERVICE))
        m_backends << new SeatManagerBackend(LOGIN1_SERVICE, LOGIN1_PATH, LOGIN1_OBJECT);

    // check if ConsoleKit2 interface exists
    if (interface->isServiceRegistered(CK2_SERVICE))
        m_backends << new SeatManagerBackend(CK2_SERVICE, CK2_PATH, CK2_OBJECT);

    // check if upower interface exists
    if (interface->isServiceRegistered(UPOWER_SERVICE))
        m_backends << new UPowerBackend(UPOWER_SERVICE, UPOWER_PATH, UPOWER_OBJECT);
}

PowerManager::~PowerManager()
{
    while (!m_backends.empty())
        delete m_backends.takeFirst();
}

PowerManager::Capabilities PowerManager::capabilities() const
{
    Capabilities caps = Capability::None;

    for (PowerManagerBackend *backend : m_backends)
        caps |= backend->capabilities();
    return caps;
}

void PowerManager::powerOff() const
{
    if (Helper::instance()->isTestMode()) {
        qDebug() << "Try powerOff in test mode";
        return;
    }

    for (PowerManagerBackend *backend : m_backends) {
        if (backend->capabilities() & Capability::PowerOff) {
            backend->powerOff();
            break;
        }
    }
}

void PowerManager::reboot() const
{
    if (Helper::instance()->isTestMode()) {
        qDebug() << "Try reboot in test mode";
        return;
    }

    for (PowerManagerBackend *backend : m_backends) {
        if (backend->capabilities() & Capability::Reboot) {
            backend->reboot();
            break;
        }
    }
}

void PowerManager::suspend() const
{
    if (Helper::instance()->isTestMode()) {
        qDebug() << "Try suspend in test mode";
        return;
    }

    for (PowerManagerBackend *backend : m_backends) {
        if (backend->capabilities() & Capability::Suspend) {
            backend->suspend();
            break;
        }
    }
}

void PowerManager::hibernate() const
{
    if (Helper::instance()->isTestMode()) {
        qDebug() << "Try hibernate in test mode";
        return;
    }

    for (PowerManagerBackend *backend : m_backends) {
        if (backend->capabilities() & Capability::Hibernate) {
            backend->hibernate();
            break;
        }
    }
}

void PowerManager::hybridSleep() const
{
    if (Helper::instance()->isTestMode()) {
        qDebug() << "Try hybridsleep in test mode";
        return;
    }

    for (PowerManagerBackend *backend : m_backends) {
        if (backend->capabilities() & Capability::HybridSleep) {
            backend->hybridSleep();
            break;
        }
    }
}
