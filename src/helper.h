// Copyright (C) 2024 UnionTech Software Technology Co., Ltd.
// Copyright (C) 2025 rewine <luhongxu@deepin.org>.
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "qmlengine.h"
#include "sessionmodel.h"
#include "usermodel.h"

#include <wglobal.h>
#include <wseat.h>

#include <QObject>
#include <QQmlApplicationEngine>

WAYLIB_SERVER_BEGIN_NAMESPACE
class WServer;
class WOutputRenderWindow;
class WCursor;
class WBackend;
class WOutput;
WAYLIB_SERVER_END_NAMESPACE

QW_BEGIN_NAMESPACE
class qw_renderer;
class qw_allocator;
QW_END_NAMESPACE

WAYLIB_SERVER_USE_NAMESPACE
QW_USE_NAMESPACE

class RootSurfaceContainer;
class Output;
class Ipc;
class SessionIpc;

class Helper : public WSeatEventFilter
{
    friend class RootSurfaceContainer;
    Q_OBJECT
    Q_PROPERTY(OutputMode outputMode READ outputMode WRITE setOutputMode NOTIFY outputModeChanged FINAL)
    Q_PROPERTY(SessionModel *sessionModel READ sessionModel CONSTANT)
    Q_PROPERTY(UserModel *userModel READ userModel CONSTANT)
    Q_PROPERTY(bool sessionInProgress READ sessionInProgress NOTIFY sessionInProgressChanged)

    QML_ELEMENT
    QML_SINGLETON

public:
    explicit Helper(QObject *parent = nullptr);
    ~Helper();

    enum class OutputMode { Copy, Extension };
    Q_ENUM(OutputMode)

    static Helper *instance();

    Q_INVOKABLE bool isTestMode() const;
    Q_INVOKABLE bool login(const QString &user, const QString &password, int sessionId);
    bool sessionInProgress() const;

    SessionModel *sessionModel() const;
    UserModel *userModel() const;
    QmlEngine *qmlEngine() const;
    WOutputRenderWindow *window() const;
    Output *output() const;
    void init();

    Output *getOutput(WOutput *output) const;

    OutputMode outputMode() const;
    void setOutputMode(OutputMode mode);

    Q_INVOKABLE void addOutput();

Q_SIGNALS:
    void primaryOutputChanged();
    void outputModeChanged();

    void sessionInProgressChanged();
    void sessionSuccess();
    void sessionError(const QString &type, const QString &description);
    void infoMessage(const QString &message);
    void errorMessage(const QString &message);

private:
    void allowNonDrmOutputAutoChangeMode(WOutput *output);
    void enableOutput(WOutput *output);

    int indexOfOutput(WOutput *output) const;

    void setCursorPosition(const QPointF &position);

    bool beforeDisposeEvent(WSeat *seat, QWindow *watched, QInputEvent *event) override;
    bool afterHandleEvent(WSeat *seat,
                          WSurface *watched,
                          QObject *surfaceItem,
                          QObject *,
                          QInputEvent *event) override;

    inline static Helper *m_instance = nullptr;

    // Greeter Backends
    SessionModel *m_sessionModel = nullptr;
    UserModel *m_userModel = nullptr;
    Ipc *m_ipc = nullptr;
    SessionIpc *m_sessionIpc = nullptr;

    // qtquick helper
    WOutputRenderWindow *m_renderWindow = nullptr;

    // wayland helper
    WServer *m_server = nullptr;
    WSeat *m_seat = nullptr;
    WBackend *m_backend = nullptr;
    qw_renderer *m_renderer = nullptr;
    qw_allocator *m_allocator = nullptr;

    // privaet data
    QList<Output *> m_outputList;

    RootSurfaceContainer *m_surfaceContainer = nullptr;
    OutputMode m_mode = OutputMode::Extension;
};

Q_DECLARE_OPAQUE_POINTER(RootSurfaceContainer *)
