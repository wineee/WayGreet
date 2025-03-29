// Copyright (C) 2013 Abdurrahman AVCI <abdurrahmanavci@gmail.com>
// Copyright (C) 2025 rewine <luhongxu@deepin.org>.
// SPDX-License-Identifier: GPL-2.0-or-later and GPL-3.0-or-later

#include "usermodel.h"

#include "wayconfig.h"

#include <QFile>
#include <QList>
#include <QStringList>
#include <QTextStream>

#include <memory>
#include <pwd.h>

#define ACCOUNTSSERVICE_DATA_DIR "/var/lib/AccountsService"

class User
{
public:
    User(const struct passwd *data, const QString icon)
        : name(QString::fromLocal8Bit(data->pw_name))
        , realName(QString::fromLocal8Bit(data->pw_gecos).split(QLatin1Char(',')).first())
        , homeDir(QString::fromLocal8Bit(data->pw_dir))
        , uid(data->pw_uid)
        , gid(data->pw_gid)
        ,
        // if shadow is used pw_passwd will be 'x' nevertheless, so this
        // will always be true
        needsPassword(strcmp(data->pw_passwd, "") != 0)
        , icon(icon)
    {
    }

    QString name;
    QString realName;
    QString homeDir;
    int uid{ 0 };
    int gid{ 0 };
    bool needsPassword{ false };
    QString icon;
};

typedef std::shared_ptr<User> UserPtr;

class UserModelPrivate
{
public:
    int lastIndex{ 0 };
    QList<UserPtr> users;
    bool containsAllUsers{ true };
};

UserModel::UserModel(bool needAllUsers, QObject *parent)
    : QAbstractListModel(parent)
    , d(new UserModelPrivate())
{
    const QString facesDir = "/usr/share/faces";  // mainConfig.Theme.FacesDir.get();
    const QString themeDir = "/usr/share/themes"; // mainConfig.Theme.ThemeDir.get();
    const QString currentTheme = "";              // mainConfig.Theme.Current.get();
    const QString themeDefaultFace =
        QStringLiteral("%1/%2/faces/.face.icon").arg(themeDir).arg(currentTheme);
    const QString defaultFace = QStringLiteral("%1/.face.icon").arg(facesDir);
    const QString iconURI =
        QStringLiteral("file://%1")
            .arg(QFile::exists(themeDefaultFace) ? themeDefaultFace : defaultFace);

    bool lastUserFound = false;

    struct passwd *current_pw;
    setpwent();
    while ((current_pw = getpwent()) != nullptr) {

        // skip entries with uids smaller than minimum uid
        if (int(current_pw->pw_uid) < WayConfig::instance()->minimumUid())
            continue;

        // skip entries with uids greater than maximum uid
        if (int(current_pw->pw_uid) > WayConfig::instance()->maximumUid())
            continue;

        // skip entries with user names in the hide users list
        if (WayConfig::instance()->hideUsers().contains(
                QString::fromLocal8Bit(current_pw->pw_name)))
            continue;

        // create user
        UserPtr user{ new User(current_pw, iconURI) };

        // add user
        d->users << user;

        if (user->name == WayConfig::instance()->lastUser())
            lastUserFound = true;

        if (!needAllUsers) {
            struct passwd *lastUserData;
            // If the theme doesn't require that all users are present, try to add the data for
            // lastUser at least
            if (!lastUserFound
                && (lastUserData = getpwnam(qPrintable(WayConfig::instance()->lastUser()))))
                d->users << UserPtr(new User(lastUserData, themeDefaultFace));

            d->containsAllUsers = false;
            break;
        }
    }

    endpwent();

    // sort users by username
    std::sort(d->users.begin(), d->users.end(), [&](const UserPtr &u1, const UserPtr &u2) {
        return u1->name < u2->name;
    });
    // Remove duplicates in case we have several sources specified
    // in nsswitch.conf(5).
    auto newEnd =
        std::unique(d->users.begin(), d->users.end(), [&](const UserPtr &u1, const UserPtr &u2) {
            return u1->name == u2->name;
        });
    d->users.erase(newEnd, d->users.end());

    bool avatarsEnabled = true;

    // find out index of the last user
    for (int i = 0; i < d->users.size(); ++i) {
        UserPtr user{ d->users.at(i) };
        if (user->name == WayConfig::instance()->lastUser())
            d->lastIndex = i;

        if (avatarsEnabled) {
            const QString userFace = QStringLiteral("%1/.face.icon").arg(user->homeDir);
            const QString systemFace =
                QStringLiteral("%1/%2.face.icon").arg(facesDir).arg(user->name);
            const QString accountsServiceFace =
                QStringLiteral(ACCOUNTSSERVICE_DATA_DIR "/icons/%1").arg(user->name);

            QString userIcon;
            // If the home is encrypted it takes a lot of time to open
            // up the greeter, therefore we try the system avatar first
            if (QFile::exists(systemFace))
                userIcon = systemFace;
            else if (QFile::exists(userFace))
                userIcon = userFace;
            else if (QFile::exists(accountsServiceFace))
                userIcon = accountsServiceFace;

            if (!userIcon.isEmpty())
                user->icon = QStringLiteral("file://%1").arg(userIcon);
        }
    }
}

UserModel::~UserModel()
{
    delete d;
}

QHash<int, QByteArray> UserModel::roleNames() const
{
    // set role names
    QHash<int, QByteArray> roleNames;
    roleNames[NameRole] = QByteArrayLiteral("name");
    roleNames[RealNameRole] = QByteArrayLiteral("realName");
    roleNames[HomeDirRole] = QByteArrayLiteral("homeDir");
    roleNames[IconRole] = QByteArrayLiteral("icon");
    roleNames[NeedsPasswordRole] = QByteArrayLiteral("needsPassword");

    return roleNames;
}

int UserModel::lastIndex() const
{
    return d->lastIndex;
}

int UserModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : d->users.length();
}

QVariant UserModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index))
        return QVariant();

    // get user
    UserPtr user = d->users[index.row()];

    // return correct value
    if (role == NameRole)
        return user->name;
    else if (role == RealNameRole)
        return user->realName;
    else if (role == HomeDirRole)
        return user->homeDir;
    else if (role == IconRole)
        return user->icon;
    else if (role == NeedsPasswordRole)
        return user->needsPassword;

    // return empty value
    return QVariant();
}

bool UserModel::containsAllUsers() const
{
    return d->containsAllUsers;
}
