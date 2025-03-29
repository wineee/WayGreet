#pragma once

#include <QObject>
#include <QUrl>

class Ipc;
class SessionIpc;
class Session;

class Backend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool sessionInProgress READ sessionInProgress NOTIFY sessionInProgressChanged)

public:
    explicit Backend(QObject *parent = nullptr);

    QUrl iconsSrc() const;
    void setIconsSrc(const QUrl &url);

    bool sessionInProgress() const;

    void setCommand(const QString &command);

    bool login(const QString &user, const QString &password, Session *session);

Q_SIGNALS:
    void userChanged();
    void sessionInProgressChanged();

    void sessionSuccess();
    void sessionError(const QString &type, const QString &description);
    void infoMessage(const QString &message);
    void errorMessage(const QString &message);

private:
    QString m_command;
    Ipc *m_ipc = nullptr;
    SessionIpc *m_session = nullptr;
};
