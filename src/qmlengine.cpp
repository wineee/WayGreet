// Copyright (C) 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qmlengine.h"

#include "output.h"

#include <woutputitem.h>

#include <QQuickItem>

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
