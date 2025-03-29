#include "backend.h"

#include "ipc.h"
#include "sessionipc.h"
#include "session.h"

#include <QDebug>
#include <QProcess>

Backend::Backend(QObject *parent)
    : QObject(parent)
    , m_ipc(new Ipc(this))
{
}

bool Backend::sessionInProgress() const
{
    return m_session;
}

void Backend::setCommand(const QString &command)
{
    m_command = command;
}

bool Backend::login(const QString &user, const QString &password, Session *session)
{
    qDebug() << Q_FUNC_INFO << session->desktopNames() << session->exec();

    if (m_session) {
        qWarning() << "Another session in progress!";
        return false;
    }

    m_session = new SessionIpc(m_ipc, session, this);
    m_session->setUsername(user);
    m_session->setPassword(password);

    connect(m_session, &SessionIpc::success, this, [this]() {
        qDebug() << "Success";
        m_session = nullptr;
        Q_EMIT sessionSuccess();
        Q_EMIT sessionInProgressChanged();
    });

    connect(m_session,
            &SessionIpc::error,
            this,
            [this](const QString &errorType, const QString &description) {
                qDebug() << "Error" << errorType << description;
                m_session = nullptr;
                Q_EMIT sessionError(errorType, description);
                Q_EMIT sessionInProgressChanged();
            });

    connect(m_session, &SessionIpc::infoMessage, this, &Backend::infoMessage);
    connect(m_session, &SessionIpc::errorMessage, this, &Backend::errorMessage);

    m_session->start();
    Q_EMIT sessionInProgressChanged();

    return true;
}
