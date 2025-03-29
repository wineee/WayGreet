#include "backend.h"
#include "ipc.h"
#include "session.h"

#include <QDebug>
#include <QProcess>

Backend::Backend(QObject *parent)
    : QObject(parent)
    , m_ipc(new Ipc(this))
{
}

QUrl Backend::iconsSrc() const
{
    return m_iconsSrc;
}

void Backend::setIconsSrc(const QUrl &url)
{
    m_iconsSrc = url;
}

bool Backend::sessionInProgress() const
{
    return m_session;
}

void Backend::setCommand(const QString &command)
{
    m_command = command;
}

bool Backend::login(const QString &user, const QString &password)
{
    if (m_session) {
        qWarning() << "Another session in progress!";
        return false;
    }

    m_session = new Session(m_ipc, this);
    m_session->setUsername(user);
    m_session->setPassword(password);
    // QProcess::splitCommand(m_command)
    m_session->setCommand(QStringList() << "sway");

    connect(m_session, &Session::success, this, [this]() {
        qDebug() << "Success";
        m_session = nullptr;
        Q_EMIT sessionSuccess();
        Q_EMIT sessionInProgressChanged();
    });

    connect(m_session, &Session::error, this, [this](const QString &errorType, const QString &description) {
        qDebug() << "Error" << errorType << description;
        m_session = nullptr;
        Q_EMIT sessionError(errorType, description);
        Q_EMIT sessionInProgressChanged();
    });

    connect(m_session, &Session::infoMessage, this, &Backend::infoMessage);
    connect(m_session, &Session::errorMessage, this, &Backend::errorMessage);

    m_session->start();
    Q_EMIT sessionInProgressChanged();

    return true;
}

// static
Backend *Backend::instance()
{
    static Backend *backend = nullptr;
    if (!backend) {
        backend = new Backend;
    }
    return backend;
}
