// Copyright (C) 2025 rewine <luhongxu@deepin.org>.
// SPDX-License-Identifier: GPL-3.0-or-later

// Thanks to https://github.com/nowrep/qmlgreet

#pragma once

#include <QObject>
#include <QVariantMap>

class QLocalSocket;

class IpcReply : public QObject
{
    Q_OBJECT

public:
    QString type() const;
    QString requestType() const;

    QString errorType() const;
    QString errorDescription() const;

    QString authMessageType() const;
    QString authMessage() const;

Q_SIGNALS:
    void finished(IpcReply *request);

private:
    explicit IpcReply(QObject *parent = nullptr);

    QVariantMap m_reply;
    QVariantMap m_request;

    friend class Ipc;
};

class Ipc : public QObject
{
    Q_OBJECT

public:
    explicit Ipc(QObject *parent = nullptr);

    IpcReply *createSession(const QString &username);
    IpcReply *postAuthMessageResponse(const QString &response = QString());
    IpcReply *startSession(const QStringList &cmd);
    IpcReply *cancelSession();

private:
    void readyRead();
    IpcReply *sendRequest(const QVariantMap &m);

    QLocalSocket *m_socket = nullptr;
    qint32 m_length = -1;
    IpcReply *m_reply = nullptr;
};
