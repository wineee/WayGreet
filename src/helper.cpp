// Copyright (C) 2023 JiDe Zhang <zhangjide@deepin.org>.
// Copyright (C) 2025 rewine <luhongxu@deepin.org>.
// SPDX-License-Identifier: GPL-3.0-or-later

// This file is modified from `tinywl` of waylib

#include "helper.h"

#include "ipc.h"
#include "sessionipc.h"
#include "qmlengine.h"
#include "rootcontainer.h"

#include <WBackend>
#include <WOutput>
#include <WServer>
#include <woutputmanagerv1.h>
#include <woutputrenderwindow.h>
#include <woutputitem.h>
#include <wquickcursor.h>
#include <wrenderhelper.h>
#include <wseat.h>

#include <qwallocator.h>
#include <qwdisplay.h>
#include <qwoutput.h>
#include <qwrenderer.h>

#include <QGuiApplication>
#include <QKeySequence>
#include <QLoggingCategory>
#include <QMouseEvent>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>

Helper::Helper(QObject *parent)
    : WSeatEventFilter(parent)
    , m_sessionModel(new SessionModel(this))
    , m_userModel(new UserModel(true, this))
    , m_ipc(new Ipc(this))
    , m_renderWindow(new WOutputRenderWindow(this))
    , m_server(new WServer(this))
    , m_rootContainer(new RootContainer(m_renderWindow->contentItem()))
{
    Q_ASSERT(!m_instance);
    m_instance = this;

    m_renderWindow->setColor(Qt::black);
    m_rootContainer->setFlag(QQuickItem::ItemIsFocusScope, true);

    connect(m_rootContainer, &RootContainer::primaryOutputChanged, this, [this] () {
        if (!m_greeter) {
            m_greeter = qmlEngine()->createGreeter(m_rootContainer->primaryOutput()->outputItem(), this);
        } else {
            m_greeter->setParentItem(m_rootContainer->primaryOutput()->outputItem());
        }
    });
}

Helper::~Helper()
{
    delete m_rootContainer;
    Q_ASSERT(m_instance == this);
    m_instance = nullptr;
}

Helper *Helper::instance()
{
    return m_instance;
}

bool Helper::isTestMode() const
{
    return !m_backend->hasDrm();
}

bool Helper::login(const QString &user, const QString &password, int sessionId)
{
    auto session = m_sessionModel->get(sessionId);
    Q_ASSERT(session);
    qDebug() << Q_FUNC_INFO << session->desktopNames() << session->exec();

    if (m_sessionIpc) {
        qWarning() << "Another session in progress!";
        return false;
    }

    m_sessionIpc = new SessionIpc(m_ipc, session, this);
    m_sessionIpc->setUsername(user);
    m_sessionIpc->setPassword(password);

    connect(m_sessionIpc, &SessionIpc::success, this, [this]() {
        qDebug() << "Success";
        m_sessionIpc = nullptr;
        Q_EMIT sessionSuccess();
        Q_EMIT sessionInProgressChanged();
    });

    connect(m_sessionIpc,
            &SessionIpc::error,
            this,
            [this](const QString &errorType, const QString &description) {
                qDebug() << "Error" << errorType << description;
                m_sessionIpc = nullptr;
                Q_EMIT sessionError(errorType, description);
                Q_EMIT sessionInProgressChanged();
            });

    connect(m_sessionIpc, &SessionIpc::infoMessage, this, &Helper::infoMessage);
    connect(m_sessionIpc, &SessionIpc::errorMessage, this, &Helper::errorMessage);

    m_sessionIpc->start();
    Q_EMIT sessionInProgressChanged();

    return true;
}

bool Helper::sessionInProgress() const
{
    return m_sessionIpc;
}

SessionModel *Helper::sessionModel() const
{
    return m_sessionModel;
}

UserModel *Helper::userModel() const
{
    return m_userModel;
}

QmlEngine *Helper::qmlEngine() const
{
    return qobject_cast<QmlEngine *>(::qmlEngine(this));
}

WOutputRenderWindow *Helper::window() const
{
    return m_renderWindow;
}

void Helper::init()
{
    auto engine = qmlEngine();
    engine->setContextForObject(m_renderWindow, engine->rootContext());
    engine->setContextForObject(m_renderWindow->contentItem(), engine->rootContext());
    // m_surfaceContainer->setQmlEngine(engine);

    m_rootContainer->init(m_server);
    m_seat = m_server->attach<WSeat>();
    m_seat->setEventFilter(this);
    m_seat->setCursor(m_rootContainer->cursor());
    m_seat->setKeyboardFocusWindow(m_renderWindow);

    m_backend = m_server->attach<WBackend>();
    connect(m_backend, &WBackend::inputAdded, this, [this](WInputDevice *device) {
        m_seat->attachInputDevice(device);
    });

    connect(m_backend, &WBackend::inputRemoved, this, [this](WInputDevice *device) {
        m_seat->detachInputDevice(device);
    });

    connect(m_backend, &WBackend::outputAdded,
            m_rootContainer, &RootContainer::onOutputAdded);

    connect(m_backend, &WBackend::outputRemoved,
            m_rootContainer, &RootContainer::onOutputRemoved);

    m_server->start();

    m_renderer = WRenderHelper::createRenderer(m_backend->handle());
    if (!m_renderer) {
        qFatal("Failed to create renderer");
    }

    m_allocator = qw_allocator::autocreate(*m_backend->handle(), *m_renderer);
    m_renderer->init_wl_display(*m_server->handle());
    m_renderWindow->init(m_renderer, m_allocator);

    m_backend->handle()->start();
}

bool Helper::beforeDisposeEvent(WSeat *seat, QWindow *, QInputEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        auto kevent = static_cast<QKeyEvent *>(event);
        if (QKeySequence(kevent->keyCombination()) == QKeySequence::Quit) {
            qApp->quit();
            return true;
        }
    }

    if (event->type() == QEvent::MouseMove || event->type() == QEvent::MouseButtonPress) {
        seat->cursor()->setVisible(true);
    } else if (event->type() == QEvent::TouchBegin) {
        seat->cursor()->setVisible(false);
    }

    return false;
}

bool Helper::afterHandleEvent(
    WSeat *seat, WSurface *watched, QObject *surfaceItem, QObject *, QInputEvent *event)
{
    Q_UNUSED(seat)

    return false;
}

void Helper::setCursorPosition(const QPointF &position)
{
    m_seat->setCursorPosition(position);
}

void Helper::addFakeOutput()
{
    qobject_cast<qw_multi_backend *>(m_backend->handle())
        ->for_each_backend(
            [](wlr_backend *backend, void *) {
                if (auto x11 = qw_x11_backend::from(backend)) {
                    qw_output::from(x11->output_create());
                } else if (auto wayland = qw_wayland_backend::from(backend)) {
                    qw_output::from(wayland->output_create());
                }
            },
            nullptr);
}

