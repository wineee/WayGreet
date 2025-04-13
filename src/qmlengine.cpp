// Copyright (C) 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qmlengine.h"

#include "output.h"

#include <woutputitem.h>

#include <QQuickItem>

Q_LOGGING_CATEGORY(qLcQmlEngine, "waygreet.qmlEngine")

QmlEngine::QmlEngine(QObject *parent)
    : QQmlApplicationEngine(parent)
    , menuBarComponent(this, "WayGreet", "OutputMenuBar")
    , greeterComponent(this, "WayGreet", "Greeter")
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
    auto context = qmlContext(parent);
    auto obj = greeterComponent.beginCreate(context);
    if (!obj) {
        qCFatal(qLcQmlEngine) << "Can't create Greeter:" << greeterComponent.errorString();
    }
    //greeterComponent.setInitialProperties(obj, { { "output", QVariant::fromValue(output) } });
    auto item = qobject_cast<QQuickItem *>(obj);
    Q_ASSERT(item);
    item->setParent(parent);
    item->setParentItem(output);
    greeterComponent.completeCreate();

    return item;
}
