// Copyright (C) 2025 rewine <luhongxu@deepin.org>.
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class Ipc;
class IpcReply;
class Session;

class SessionIpc : public QObject
{
    Q_OBJECT

public:
    explicit SessionIpc(Ipc *ipc, Session *session, QObject *parent = nullptr);

    void setUsername(const QString &username);
    void setPassword(const QString &password);

    void start();

Q_SIGNALS:
    void success();
    void error(const QString &errorType, const QString &description);
    void infoMessage(const QString &message);
    void errorMessage(const QString &message);

private:
    void addRequest(IpcReply *reply);
    void replyFinished(IpcReply *reply);

    Ipc *m_ipc { nullptr };
    Session *m_session { nullptr };
    QString m_username;
    QString m_password;
};
