// Copyright (C) 2015-2016 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// Copyright (C) 2013 Abdurrahman AVCI <abdurrahmanavci@gmail.com>
// Copyright (C) 2025 rewine <luhongxu@deepin.org>
// SPDX-License-Identifier: GPL-2.0-or-later and GPL-3.0-or-later

#include "sessionmodel.h"

#include "wayconfig.h"

#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QProcessEnvironment>
#include <QVector>

class SessionModelPrivate
{
public:
    ~SessionModelPrivate()
    {
        qDeleteAll(sessions);
        sessions.clear();
    }

    int lastIndex{ 0 };
    QStringList displayNames;
    QVector<Session *> sessions;
};

SessionModel::SessionModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new SessionModelPrivate())
{
    // initial population
    beginResetModel();
    populate(Session::WaylandSession, WayConfig::instance()->waylandSessionDir());
    if (WayConfig::instance()->showX11Session())
        populate(Session::X11Session, WayConfig::instance()->x11SessionDir());
    endResetModel();

    // refresh everytime a file is changed, added or removed
    QFileSystemWatcher *watcher = new QFileSystemWatcher(this);
    connect(watcher, &QFileSystemWatcher::directoryChanged, [this]() {
        // Recheck for flag to show Wayland sessions
        beginResetModel();
        d->sessions.clear();
        d->displayNames.clear();
        populate(Session::WaylandSession, WayConfig::instance()->waylandSessionDir());
        if (WayConfig::instance()->showX11Session())
            populate(Session::X11Session, WayConfig::instance()->x11SessionDir());
        endResetModel();
    });
    watcher->addPaths(WayConfig::instance()->waylandSessionDir());
    if (WayConfig::instance()->showX11Session())
        watcher->addPaths(WayConfig::instance()->x11SessionDir());
}

SessionModel::~SessionModel()
{
    delete d;
}

QHash<int, QByteArray> SessionModel::roleNames() const
{
    // set role names
    QHash<int, QByteArray> roleNames;
    roleNames[DirectoryRole] = QByteArrayLiteral("directory");
    roleNames[FileRole] = QByteArrayLiteral("file");
    roleNames[TypeRole] = QByteArrayLiteral("type");
    roleNames[NameRole] = QByteArrayLiteral("name");
    roleNames[ExecRole] = QByteArrayLiteral("exec");
    roleNames[CommentRole] = QByteArrayLiteral("comment");

    return roleNames;
}

int SessionModel::lastIndex() const
{
    return d->lastIndex;
}

void SessionModel::setLastIndex(int index)
{
    if (const auto *session = get(index))
        WayConfig::instance()->setLastSession(session->fileName());
}

int SessionModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : d->sessions.length();
}

Session *SessionModel::get(int index)
{
    if (index < 0 || index >= d->sessions.length())
        return nullptr;
    return d->sessions[index];
}

QVariant SessionModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index))
        return QVariant();

    // get session
    const auto *session = d->sessions[index.row()];

    // return correct value
    switch (role) {
    case DirectoryRole:
        return session->directory().absolutePath();
    case FileRole:
        return session->fileName();
    case TypeRole:
        return session->type();
    case NameRole:
        if (d->displayNames.count(session->displayName()) > 1
            && session->type() == Session::WaylandSession)
            return tr("%1 (Wayland)").arg(session->displayName());
        return session->displayName();
    case ExecRole:
        return session->exec();
    case CommentRole:
        return session->comment();
    default:
        break;
    }

    // return empty value
    return QVariant();
}

void SessionModel::populate(Session::Type type, const QStringList &dirPaths)
{
    // read session files
    QStringList sessions;
    for (const auto &path : dirPaths) {
        QDir dir = path;
        dir.setNameFilters(QStringList() << QStringLiteral("*.desktop"));
        dir.setFilter(QDir::Files);
        sessions += dir.entryList();
    }
    // read session
    sessions.removeDuplicates();
    for (auto &&session : std::as_const(sessions)) {
        qDebug() << "Found Session: " << sessions;
        auto *si = new Session(type, session);
        bool execAllowed = true;
        QFileInfo fi(si->tryExec());
        if (fi.isAbsolute()) {
            if (!fi.exists() || !fi.isExecutable())
                execAllowed = false;
        } else {
            execAllowed = false;
            QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
            QString envPath = env.value(QStringLiteral("PATH"));
            const QStringList pathList = envPath.split(QLatin1Char(':'));
            for (const QString &path : pathList) {
                QDir pathDir(path);
                fi.setFile(pathDir, si->tryExec());
                if (fi.exists() && fi.isExecutable()) {
                    execAllowed = true;
                    break;
                }
            }
        }
        // add to sessions list
        if (!si->isHidden() && !si->isNoDisplay() && execAllowed) {
            d->displayNames.append(si->displayName());
            d->sessions.push_back(si);
        } else {
            delete si;
        }
    }

    // find out index of the last session
    for (int i = 0; i < d->sessions.size(); ++i) {
        if (d->sessions.at(i)->fileName() == WayConfig::instance()->lastSession()) {
            d->lastIndex = i;
            break;
        }
    }
}
