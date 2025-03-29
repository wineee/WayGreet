// Copyright (C) 2013 Abdurrahman AVCI <abdurrahmanavci@gmail.com>
// Copyright (C) 2025 rewine <luhongxu@deepin.org>.
// SPDX-License-Identifier: GPL-2.0-or-later and GPL-3.0-or-later

// This file is modified from `sddm`

#pragma once

#include <QAbstractListModel>
#include <QHash>

class UserModelPrivate;

class UserModel : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY(UserModel)
    Q_PROPERTY(int lastIndex READ lastIndex CONSTANT)
    Q_PROPERTY(int count READ rowCount CONSTANT)
    Q_PROPERTY(bool containsAllUsers READ containsAllUsers CONSTANT)
public:
    enum UserRoles {
        NameRole = Qt::UserRole + 1,
        RealNameRole,
        HomeDirRole,
        IconRole,
        NeedsPasswordRole
    };
    Q_ENUM(UserRoles)

    UserModel(bool needAllUsers, QObject *parent = 0);
    ~UserModel();

    QHash<int, QByteArray> roleNames() const override;

    int lastIndex() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool containsAllUsers() const;

private:
    UserModelPrivate *d{ nullptr };
};
