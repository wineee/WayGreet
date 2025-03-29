// Copyright (C) 2025 rewine <luhongxu@deepin.org>.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sessionipc.h"
#include "session.h"

#include "ipc.h"

#include <QDebug>

SessionIpc::SessionIpc(Ipc *ipc, Session *session, QObject *parent)
    : QObject(parent)
    , m_ipc(ipc)
    , m_session(session)
{
}

void SessionIpc::setUsername(const QString &username)
{
    m_username = username;
}

void SessionIpc::setPassword(const QString &password)
{
    m_password = password;
}

void SessionIpc::start()
{
    qInfo() << "Start session" << m_username << QString('*').repeated(m_password.size());
    addRequest(m_ipc->createSession(m_username));
}

void SessionIpc::addRequest(IpcReply *reply)
{
    connect(reply, &IpcReply::finished, this, &SessionIpc::replyFinished);
}

void SessionIpc::replyFinished(IpcReply *reply)
{
    if (reply->type() == QLatin1String("error")) {
        qWarning() << reply->requestType() << "error" << reply->errorType()
                   << reply->errorDescription();
        Q_EMIT error(reply->errorType(), reply->errorDescription());
        addRequest(m_ipc->cancelSession());
        return;
    }

    if (reply->type() == QLatin1String("success")) {
        if (reply->requestType() == QLatin1String("create_session")
            || reply->requestType() == QLatin1String("post_auth_message_response")) {
            auto command = QProcess::splitCommand(m_session->exec());
            addRequest(m_ipc->startSession(command));
            return;
        }
        if (reply->requestType() == QLatin1String("start_session")) {
            Q_EMIT success();
        }
        deleteLater();
        return;
    }

    if (reply->type() == QLatin1String("auth_message")) {
        if (reply->authMessageType() == QLatin1String("visible")) {
            qWarning() << "Unhandled visible auth message type!";
            Q_EMIT error(QStringLiteral("not_implemented"),
                         QStringLiteral("Auth message 'visible' not implemented."));
            addRequest(m_ipc->cancelSession());
        } else if (reply->authMessageType() == QLatin1String("secret")) {
            addRequest(m_ipc->postAuthMessageResponse(m_password));
        } else if (reply->authMessageType() == QLatin1String("info")) {
            qInfo() << "Info" << reply->authMessage();
            Q_EMIT infoMessage(reply->authMessage());
            // FIXME: What now?
        } else if (reply->authMessageType() == QLatin1String("error")) {
            qInfo() << "Error" << reply->authMessage();
            Q_EMIT errorMessage(reply->authMessage());
            // FIXME: What now?
        }
        return;
    }
}
