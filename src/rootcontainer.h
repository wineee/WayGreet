// Copyright (C) 2024 UnionTech Software Technology Co., Ltd.
// Copyright (C) 2025 rewine <luhongxu@deepin.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "output.h"

#include <wglobal.h>
#include <WOutput>

#include <QQuickItem>

WAYLIB_SERVER_BEGIN_NAMESPACE
class WSurface;
class WSurfaceItem;
class WOutputLayout;
class WCursor;
WAYLIB_SERVER_END_NAMESPACE

WAYLIB_SERVER_USE_NAMESPACE

class RootContainer : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(WAYLIB_SERVER_NAMESPACE::WOutputLayout *outputLayout READ outputLayout CONSTANT FINAL)
    Q_PROPERTY(WAYLIB_SERVER_NAMESPACE::WCursor *cursor READ cursor CONSTANT FINAL)
    Q_PROPERTY(Output *primaryOutput READ primaryOutput WRITE setPrimaryOutput NOTIFY primaryOutputChanged FINAL)

public:
    enum class OutputMode { Copy, Extension };
    Q_ENUM(OutputMode)

    explicit RootContainer(QQuickItem *parent);

    void init(WServer *server);

    WOutputLayout *outputLayout() const;
    WCursor *cursor() const;

    Output *cursorOutput() const;
    Output *primaryOutput() const;
    void setPrimaryOutput(Output *newPrimaryOutput);
    const QList<Output *> &outputs() const;

    void enableOutput(WOutput *output);
    int indexOfOutput(WOutput *output) const;
    Output *getOutput(WOutput *output) const;

    OutputMode outputMode() const;
    void setOutputMode(OutputMode mode);

public Q_SLOTS:
    void onOutputAdded(WOutput *output);
    void onOutputRemoved(WOutput *output);

Q_SIGNALS:
    void primaryOutputChanged();

private:
    void addOutput(Output *output);
    void removeOutput(Output *output);

    void ensureCursorVisible();
    void allowNonDrmOutputAutoChangeMode(WOutput *output);

    QList<Output *> m_outputList;
    QPointer<Output> m_primaryOutput;
    WOutputLayout *m_outputLayout = nullptr;
    WCursor *m_cursor = nullptr;
    OutputMode m_mode = OutputMode::Extension;
};

Q_DECLARE_OPAQUE_POINTER(WAYLIB_SERVER_NAMESPACE::WOutputLayout *)
Q_DECLARE_OPAQUE_POINTER(WAYLIB_SERVER_NAMESPACE::WCursor *)
