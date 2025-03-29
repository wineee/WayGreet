// Copyright (C) 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "output.h"

#include <wglobal.h>

#include <QQuickItem>

WAYLIB_SERVER_BEGIN_NAMESPACE
class WSurface;
class WSurfaceItem;
class WOutputLayout;
class WCursor;
WAYLIB_SERVER_END_NAMESPACE

WAYLIB_SERVER_USE_NAMESPACE

class RootSurfaceContainer : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(WAYLIB_SERVER_NAMESPACE::WOutputLayout *outputLayout READ outputLayout CONSTANT FINAL)
    Q_PROPERTY(WAYLIB_SERVER_NAMESPACE::WCursor *cursor READ cursor CONSTANT FINAL)
    Q_PROPERTY(Output *primaryOutput READ primaryOutput WRITE setPrimaryOutput NOTIFY primaryOutputChanged FINAL)

public:
    explicit RootSurfaceContainer(QQuickItem *parent);

    void init(WServer *server);

    WOutputLayout *outputLayout() const;
    WCursor *cursor() const;

    Output *cursorOutput() const;
    Output *primaryOutput() const;
    void setPrimaryOutput(Output *newPrimaryOutput);
    const QList<Output *> &outputs() const;

    void addOutput(Output *output);
    void removeOutput(Output *output);
signals:
    void primaryOutputChanged();
    void moveResizeFinised();

private:
    void ensureCursorVisible();

    WOutputLayout *m_outputLayout = nullptr;
    QList<Output *> m_outputList;
    QPointer<Output> m_primaryOutput;
    WCursor *m_cursor = nullptr;
};

Q_DECLARE_OPAQUE_POINTER(WAYLIB_SERVER_NAMESPACE::WOutputLayout *)
Q_DECLARE_OPAQUE_POINTER(WAYLIB_SERVER_NAMESPACE::WCursor *)
