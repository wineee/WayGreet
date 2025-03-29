// Copyright (C) 2015-2016 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// Copyright (C) 2013 Abdurrahman AVCI <abdurrahmanavci@gmail.com>
// Copyright (C) 2025 rewine <luhongxu@deepin.org>
// SPDX-License-Identifier: GPL-2.0-or-later and GPL-3.0-or-later

// This file is modified from `sddm`

#pragma once

#include "session.h"

#include <QAbstractListModel>
#include <QHash>

class SessionModelPrivate;

class SessionModel : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY(SessionModel)
    Q_PROPERTY(int lastIndex READ lastIndex CONSTANT)
    Q_PROPERTY(int count READ rowCount CONSTANT)

public:
    enum SessionRole {
        DirectoryRole = Qt::UserRole + 1,
        FileRole,
        TypeRole,
        NameRole,
        ExecRole,
        CommentRole
    };
    Q_ENUM(SessionRole)

    SessionModel(QObject *parent = 0);
    ~SessionModel();

    QHash<int, QByteArray> roleNames() const override;

    int lastIndex() const;
    Q_INVOKABLE void setLastIndex(int index);
    Session *get(int indec);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    SessionModelPrivate *d{ nullptr };

    void populate(Session::Type type, const QStringList &dirPaths);
};
