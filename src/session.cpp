// Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// Copyright (C) 2025 rewine <luhongxu@deepin.org>.
// SPDX-License-Identifier: GPL-2.0-or-later and GPL-3.0-or-later

#include "session.h"
#include "wayconfig.h"

#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QRegularExpression>
#include <QSettings>
#include <QTextStream>
#include <QStringView>
#include <QtGlobal>

const QString s_entryExtention = QStringLiteral(".desktop");

// QSettings::IniFormat can't be used to read .desktop files due to different
// syntax of values (escape sequences, quoting, automatic QStringList detection).
// So implement yet another .desktop file parser.
class DesktopFileFormat
{
    static bool readFunc(QIODevice &device, QSettings::SettingsMap &map)
    {
        QString filename = QStringLiteral("(unknown)");
        if (QFile *file = qobject_cast<QFile *>(&device); file)
            filename = file->fileName();

        QString currentSectionName;
        for (int lineNumber = 1; !device.atEnd(); lineNumber++) {
            // Iterate each line, remove line terminators
            const auto line = device.readLine().replace("\r", "").replace("\n", "");
            if (line.isEmpty() || line.startsWith('#'))
                continue; // Ignore empty lines and comments

            if (line.startsWith('[')) // Section header
            {
                const int endOfHeader = line.lastIndexOf(']');
                if (endOfHeader < 0) {
                    qWarning() << QStringLiteral("%1:%2: Invalid section header")
                                      .arg(filename)
                                      .arg(lineNumber);
                    return false;
                }
                if (endOfHeader != line.length() - 1)
                    qWarning() << QStringLiteral("%1:%2: Section header does not end line with ]")
                                      .arg(filename)
                                      .arg(lineNumber);

                currentSectionName = QString::fromUtf8(line.mid(1, endOfHeader - 1));
            } else if (int equalsPos = line.indexOf('='); equalsPos > 0) // Key=Value
            {
                const auto key = QString::fromUtf8(line.left(equalsPos));

                // Read the value, handle escape sequences
                auto valueBytes = line.mid(equalsPos + 1);
                valueBytes.replace("\\s", " ").replace("\\n", "\n");
                valueBytes.replace("\\t", "\t").replace("\\r", "\r");
                valueBytes.replace("\\\\", "\\");

                auto value = QString::fromUtf8(valueBytes);
                map.insert(currentSectionName + QLatin1Char('/') + key, value);
            }
        }

        return true;
    }

public:
    // Register the .desktop file format if necessary, return its id.
    static QSettings::Format format()
    {
        static QSettings::Format s_format = QSettings::InvalidFormat;
        if (s_format == QSettings::InvalidFormat)
            s_format = QSettings::registerFormat(QStringLiteral("desktop"),
                                                 DesktopFileFormat::readFunc,
                                                 nullptr,
                                                 Qt::CaseSensitive);

        return s_format;
    }
};

Session::Session()
    : m_valid(false)
    , m_type(UnknownSession)
    , m_isHidden(false)
    , m_isNoDisplay(false)
{
}

Session::Session(Type type, const QString &fileName)
    : Session()
{
    setTo(type, fileName);
}

bool Session::isValid() const
{
    return m_valid;
}

Session::Type Session::type() const
{
    return m_type;
}

int Session::vt() const
{
    return m_vt;
}

void Session::setVt(int vt)
{
    m_vt = vt;
}

QString Session::xdgSessionType() const
{
    return m_xdgSessionType;
}

QDir Session::directory() const
{
    return m_dir;
}

QString Session::fileName() const
{
    return m_fileName;
}

QString Session::displayName() const
{
    return m_displayName;
}

QString Session::comment() const
{
    return m_comment;
}

QString Session::exec() const
{
    return m_exec;
}

QString Session::tryExec() const
{
    return m_tryExec;
}

QString Session::desktopSession() const
{
    return QFileInfo(m_fileName).completeBaseName();
}

QString Session::desktopNames() const
{
    return m_desktopNames;
}

bool Session::isHidden() const
{
    return m_isHidden;
}

bool Session::isNoDisplay() const
{
    return m_isNoDisplay;
}

QProcessEnvironment Session::additionalEnv() const
{
    return m_additionalEnv;
}

void Session::setTo(Type type, const QString &_fileName)
{
    QString fileName(_fileName);
    if (!fileName.endsWith(s_entryExtention))
        fileName += s_entryExtention;

    QFileInfo info(fileName);

    m_type = UnknownSession;
    m_valid = false;
    m_desktopNames.clear();

    QStringList SessionDirs;

    switch (type) {
    case WaylandSession:
        SessionDirs = WayConfig::instance()->waylandSessionDir();
        m_xdgSessionType = QStringLiteral("wayland");
        break;
    case X11Session:
        SessionDirs = WayConfig::instance()->x11SessionDir();
        m_xdgSessionType = QStringLiteral("x11");
        break;
    default:
        m_xdgSessionType.clear();
        break;
    }

    QFile file;
    for (const auto &path : std::as_const(SessionDirs)) {
        m_dir.setPath(path);
        m_fileName = m_dir.absoluteFilePath(fileName);

        qDebug() << "Reading from" << m_fileName;

        file.setFileName(m_fileName);
        if (file.open(QIODevice::ReadOnly))
            break;
    }
    if (!file.isOpen())
        return;

    QSettings settings(m_fileName, DesktopFileFormat::format());
    QStringList locales = { QLocale().name() };
    if (auto clean = QLocale().name().remove(QRegularExpression(QLatin1String("_.*")));
        clean != locales.constFirst()) {
        locales << clean;
    }

    if (settings.status() != QSettings::NoError)
        return;

    settings.beginGroup(QLatin1String("Desktop Entry"));

    auto localizedValue = [&](const QLatin1String &key) {
        for (QString locale : std::as_const(locales)) {
            QString localizedValue =
                settings.value(key + QLatin1Char('[') + locale + QLatin1Char(']'), QString())
                    .toString();
            if (!localizedValue.isEmpty()) {
                return localizedValue;
            }
        }
        return settings.value(key).toString();
    };

    m_displayName = localizedValue(QLatin1String("Name"));
    m_comment = localizedValue(QLatin1String("Comment"));
    m_exec = settings.value(QLatin1String("Exec"), QString()).toString();
    m_tryExec = settings.value(QLatin1String("TryExec"), QString()).toString();
    m_desktopNames = settings.value(QLatin1String("DesktopNames"), QString())
                         .toString()
                         .replace(QLatin1Char(';'), QLatin1Char(':'));
    QString hidden = settings.value(QLatin1String("Hidden"), QString()).toString();
    m_isHidden = hidden.toLower() == QLatin1String("true");
    QString noDisplay = settings.value(QLatin1String("NoDisplay"), QString()).toString();
    m_isNoDisplay = noDisplay.toLower() == QLatin1String("true");
    QString additionalEnv = settings.value(QLatin1String("X-SDDM-Env"), QString()).toString();
    m_additionalEnv = parseEnv(additionalEnv);
    settings.endGroup();

    m_type = type;
    m_valid = true;
}

Session &Session::operator=(const Session &other)
{
    setTo(other.type(), other.fileName());
    return *this;
}

QProcessEnvironment Session::parseEnv(const QString &list)
{
    QProcessEnvironment env;
    const auto entryList = QStringView{ list }.split(u',', Qt::SkipEmptyParts);
    for (const auto &entry : entryList) {
        int midPoint = entry.indexOf(QLatin1Char('='));
        if (midPoint < 0) {
            qWarning() << "Malformed entry in" << fileName() << ":" << entry;
            continue;
        }
        env.insert(entry.left(midPoint).toString(), entry.mid(midPoint + 1).toString());
    }
    return env;
}
