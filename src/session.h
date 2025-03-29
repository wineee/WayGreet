// Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// Copyright (C) 2025 rewine <luhongxu@deepin.org>.
// SPDX-License-Identifier: GPL-2.0-or-later and GPL-3.0-or-later

// This file is modified from `sddm`

#pragma once

#include <QDataStream>
#include <QDir>
#include <QProcessEnvironment>
#include <QSharedPointer>

class Session
{
public:
    enum Type { UnknownSession = 0, X11Session, WaylandSession };

    explicit Session();
    Session(Type type, const QString &fileName);

    bool isValid() const;

    Type type() const;

    int vt() const;
    void setVt(int vt);

    QString xdgSessionType() const;

    QDir directory() const;
    QString fileName() const;

    QString displayName() const;
    QString comment() const;

    QString exec() const;
    QString tryExec() const;

    QString desktopSession() const;
    QString desktopNames() const;

    bool isHidden() const;
    bool isNoDisplay() const;

    QProcessEnvironment additionalEnv() const;

    void setTo(Type type, const QString &name);

    Session &operator=(const Session &other);

private:
    QProcessEnvironment parseEnv(const QString &list);
    bool m_valid;
    Type m_type;
    int m_vt = 0;
    QDir m_dir;
    QString m_name;
    QString m_fileName;
    QString m_displayName;
    QString m_comment;
    QString m_exec;
    QString m_tryExec;
    QString m_xdgSessionType;
    QString m_desktopNames;
    QProcessEnvironment m_additionalEnv;
    bool m_isHidden;
    bool m_isNoDisplay;
};

inline QDataStream &operator<<(QDataStream &stream, const Session &WSession)
{
    stream << quint32(WSession.type()) << WSession.fileName();
    return stream;
}

inline QDataStream &operator>>(QDataStream &stream, Session &WSession)
{
    quint32 type;
    QString fileName;
    stream >> type >> fileName;
    WSession.setTo(static_cast<Session::Type>(type), fileName);
    return stream;
}
