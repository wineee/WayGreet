// Copyright (C) 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

#include "qmlengine.h"
#include "backend.h"
#include "powermanager.h"
#include "sessionmodel.h"
#include "usermodel.h"

#include <wglobal.h>
#include <wqmlcreator.h>
#include <wseat.h>

#include <QObject>
#include <QQmlApplicationEngine>

WAYLIB_SERVER_BEGIN_NAMESPACE
class WServer;
class WOutputRenderWindow;
class WOutputLayout;
class WCursor;
class WBackend;
class WOutputItem;
class WOutputViewport;
class WOutputLayer;
class WOutput;
WAYLIB_SERVER_END_NAMESPACE

QW_BEGIN_NAMESPACE
class qw_renderer;
class qw_allocator;
class qw_compositor;
QW_END_NAMESPACE

WAYLIB_SERVER_USE_NAMESPACE
QW_USE_NAMESPACE

class RootSurfaceContainer;
class Output;
class Helper : public WSeatEventFilter
{
    friend class RootSurfaceContainer;
    Q_OBJECT
    Q_PROPERTY(OutputMode outputMode READ outputMode WRITE setOutputMode NOTIFY outputModeChanged FINAL)
    Q_PROPERTY(SessionModel *sessionModel READ sessionModel CONSTANT)
    Q_PROPERTY(UserModel *userModel READ userModel CONSTANT)
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit Helper(QObject *parent = nullptr);
    ~Helper();

    enum class OutputMode {
        Copy,
        Extension
    };
    Q_ENUM(OutputMode)

    static Helper *instance();

    SessionModel *sessionModel() const;
    UserModel *userModel() const;
    QmlEngine *qmlEngine() const;
    WOutputRenderWindow *window() const;
    Output* output() const;
    void init();

    Output *getOutput(WOutput *output) const;

    OutputMode outputMode() const;
    void setOutputMode(OutputMode mode);

    Q_INVOKABLE void addOutput();

signals:
    void primaryOutputChanged();
    void outputModeChanged();

private:
    void allowNonDrmOutputAutoChangeMode(WOutput *output);
    void enableOutput(WOutput *output);

    int indexOfOutput(WOutput *output) const;

    void setCursorPosition(const QPointF &position);

    bool beforeDisposeEvent(WSeat *seat, QWindow *watched, QInputEvent *event) override;
    bool afterHandleEvent(WSeat *seat, WSurface *watched, QObject *surfaceItem, QObject *, QInputEvent *event) override;

    static Helper *m_instance;

    // Greeter Backends
    Backend *m_greetd = nullptr;
    PowerManager *m_powerManager = nullptr;
    SessionModel *m_sessionModel = nullptr;
    UserModel *m_userModel = nullptr;

    // qtquick helper
    WOutputRenderWindow *m_renderWindow = nullptr;

    // wayland helper
    WServer *m_server = nullptr;
    WSeat *m_seat = nullptr;
    WBackend *m_backend = nullptr;
    qw_renderer *m_renderer = nullptr;
    qw_allocator *m_allocator = nullptr;

    // privaet data
    QList<Output*> m_outputList;

    RootSurfaceContainer *m_surfaceContainer = nullptr;
    OutputMode m_mode = OutputMode::Extension;
};

Q_DECLARE_OPAQUE_POINTER(RootSurfaceContainer*)
