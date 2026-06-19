// Copyright (C) 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qmlengine.h"

#include "output.h"
#include "wayconfig.h"

#include <woutputitem.h>

#include <QFile>
#include <QQuickItem>
#include <QStandardPaths>
#include <QDir>
#include <QQmlPropertyMap>
#include <QSettings>
#include <QQmlContext>
#include <QFileInfo>

class ThemeConfig : public QQmlPropertyMap
{
    Q_OBJECT
public:
    explicit ThemeConfig(const QString &confPath, QObject *parent = nullptr)
        : QQmlPropertyMap(this, parent)
    {
        if (QFile::exists(confPath)) {
            QSettings settings(confPath, QSettings::IniFormat);
            settings.beginGroup("General");
            for (const QString &key : settings.childKeys()) {
                insert(key, settings.value(key));
            }
        }
    }

    Q_INVOKABLE QString stringValue(const QString &key) const {
        return value(key).toString();
    }
    Q_INVOKABLE bool boolValue(const QString &key) const {
        return value(key).toBool();
    }
    Q_INVOKABLE int intValue(const QString &key) const {
        return value(key).toInt();
    }
    Q_INVOKABLE qreal realValue(const QString &key) const {
        return value(key).toReal();
    }
};


Q_LOGGING_CATEGORY(qLcQmlEngine, "waygreet.qmlEngine")

QmlEngine::QmlEngine(QObject *parent)
    : QQmlApplicationEngine(parent)
    , menuBarComponent(this, "WayGreet", "OutputMenuBar")
{

}

QQuickItem *QmlEngine::createMenuBar(WOutputItem *output, QQuickItem *parent)
{
    auto context = qmlContext(parent);
    auto obj = menuBarComponent.beginCreate(context);
    menuBarComponent.setInitialProperties(obj, { { "output", QVariant::fromValue(output) } });
    auto item = qobject_cast<QQuickItem *>(obj);
    Q_ASSERT(item);
    item->setParent(parent);
    item->setParentItem(parent);
    menuBarComponent.completeCreate();

    return item;
}

QQuickItem *QmlEngine::createGreeter(WOutputItem *output, QObject *parent)
{
    if (!greeterComponent) {
        greeterComponent = new QQmlComponent(this);
        auto themeName = WayConfig::instance()->theme();
        QString themePath;

        if (!themeName.isEmpty()) {
            auto customThemeDir = WayConfig::instance()->themeDir();

            if (!customThemeDir.isEmpty()) {
                themePath = QDir(customThemeDir).filePath(themeName + "/Main.qml");
            } else if (themeName.startsWith("/")) {
                themePath = themeName + "/Main.qml";
            } else {
                QString relPath = QStringLiteral("waygreet/themes/%1/Main.qml").arg(themeName);
                themePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, relPath);
            }
        }

        if (!themePath.isEmpty()) {
            QFileInfo fi(themePath);
            QString confPath = fi.dir().filePath("theme.conf");
            rootContext()->setContextProperty("config", new ThemeConfig(confPath, this));

            if (QFile::exists(themePath)) {
                greeterComponent->loadUrl(QUrl::fromLocalFile(themePath));
            } else {
                qCWarning(qLcQmlEngine) << "Theme file not found for:" << themeName << "fallback to default";
            }
        } else {
            rootContext()->setContextProperty("config", new ThemeConfig("", this));
        }

        if (greeterComponent->isNull() || greeterComponent->isError()) {
            if (greeterComponent->isError()) {
                qCWarning(qLcQmlEngine) << "Theme load error:" << greeterComponent->errorString();
            }
            greeterComponent->loadFromModule("WayGreet", "Greeter");
        }
    }

    auto context = qmlContext(parent);
    auto obj = greeterComponent->beginCreate(context);
    if (!obj) {
        qCFatal(qLcQmlEngine) << "Can't create Greeter:" << greeterComponent->errorString();
    }
    //greeterComponent->setInitialProperties(obj, { { "output", QVariant::fromValue(output) } });
    auto item = qobject_cast<QQuickItem *>(obj);
    Q_ASSERT(item);
    item->setParent(parent);
    item->setParentItem(output);
    greeterComponent->completeCreate();

    return item;
}

#include "qmlengine.moc"
