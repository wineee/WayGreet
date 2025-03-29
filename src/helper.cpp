// Copyright (C) 2023 JiDe Zhang <zhangjide@deepin.org>.
// Copyright (C) 2025 rewine <luhongxu@deepin.org>.
// SPDX-License-Identifier: GPL-3.0-or-later

// This file is modified from `tinywl` of waylib

#include "helper.h"

#include "ipc.h"
#include "sessionipc.h"
#include "output.h"
#include "qmlengine.h"
#include "rootsurfacecontainer.h"

#include <WBackend>
#include <WOutput>
#include <WServer>
#include <woutputitem.h>
#include <woutputlayout.h>
#include <woutputmanagerv1.h>
#include <woutputrenderwindow.h>
#include <woutputviewport.h>
#include <wquickcursor.h>
#include <wrenderhelper.h>
#include <wseat.h>

#include <qwallocator.h>
#include <qwdisplay.h>
#include <qwlogging.h>
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

#define WLR_FRACTIONAL_SCALE_V1_VERSION 1

Helper *Helper::m_instance = nullptr;

Helper::Helper(QObject *parent)
    : WSeatEventFilter(parent)
    , m_sessionModel(new SessionModel(this))
    , m_userModel(new UserModel(true, this))
    , m_ipc(new Ipc(this))
    , m_renderWindow(new WOutputRenderWindow(this))
    , m_server(new WServer(this))
    , m_surfaceContainer(new RootSurfaceContainer(m_renderWindow->contentItem()))
{
    Q_ASSERT(!m_instance);
    m_instance = this;

    m_renderWindow->setColor(Qt::black);
    m_surfaceContainer->setFlag(QQuickItem::ItemIsFocusScope, true);
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    m_surfaceContainer->setFocusPolicy(Qt::StrongFocus);
#endif
}

Helper::~Helper()
{
    delete m_surfaceContainer;
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

    m_surfaceContainer->init(m_server);
    m_seat = m_server->attach<WSeat>();
    m_seat->setEventFilter(this);
    m_seat->setCursor(m_surfaceContainer->cursor());
    m_seat->setKeyboardFocusWindow(m_renderWindow);

    m_backend = m_server->attach<WBackend>();
    connect(m_backend, &WBackend::inputAdded, this, [this](WInputDevice *device) {
        m_seat->attachInputDevice(device);
    });

    connect(m_backend, &WBackend::inputRemoved, this, [this](WInputDevice *device) {
        m_seat->detachInputDevice(device);
    });

    connect(m_backend, &WBackend::outputAdded, this, [this](WOutput *output) {
        allowNonDrmOutputAutoChangeMode(output);
        Output *o;
        if (m_mode == OutputMode::Extension || !m_surfaceContainer->primaryOutput()) {
            o = Output::createPrimary(output, qmlEngine(), this);
            o->outputItem()->stackBefore(m_surfaceContainer);
            m_surfaceContainer->addOutput(o);
        } else if (m_mode == OutputMode::Copy) {
            o = Output::createCopy(output, m_surfaceContainer->primaryOutput(), qmlEngine(), this);
        }

        m_outputList.append(o);
        enableOutput(output);
    });

    connect(m_backend, &WBackend::outputRemoved, this, [this](WOutput *output) {
        auto index = indexOfOutput(output);
        Q_ASSERT(index >= 0);
        const auto o = m_outputList.takeAt(index);
        m_surfaceContainer->removeOutput(o);
        delete o;
    });

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

void Helper::allowNonDrmOutputAutoChangeMode(WOutput *output)
{
    output->safeConnect(&qw_output::notify_request_state,
                        this,
                        [this](wlr_output_event_request_state *newState) {
                            if (newState->state->committed & WLR_OUTPUT_STATE_MODE) {
                                auto output = qobject_cast<qw_output *>(sender());
                                output->commit_state(newState->state);
                            }
                        });
}

void Helper::enableOutput(WOutput *output)
{
    // Enable on default
    auto qwoutput = output->handle();
    // Don't care for WOutput::isEnabled, must do WOutput::commit here,
    // In order to ensure trigger QWOutput::frame signal, WOutputRenderWindow
    // needs this signal to render next frmae. Because QWOutput::frame signal
    // maybe emit before WOutputRenderWindow::attach, if no commit here,
    // WOutputRenderWindow will ignore this ouptut on render.
    if (!qwoutput->property("_Enabled").toBool()) {
        qwoutput->setProperty("_Enabled", true);
        qw_output_state newState;

        if (!qwoutput->handle()->current_mode) {
            auto mode = qwoutput->preferred_mode();
            if (mode)
                newState.set_mode(mode);
        }
        newState.set_enabled(true);
        bool ok = qwoutput->commit_state(newState);
        Q_ASSERT(ok);
    }
}

int Helper::indexOfOutput(WOutput *output) const
{
    for (int i = 0; i < m_outputList.size(); i++) {
        if (m_outputList.at(i)->output() == output)
            return i;
    }
    return -1;
}

Output *Helper::getOutput(WOutput *output) const
{
    for (auto o : std::as_const(m_outputList)) {
        if (o->output() == output)
            return o;
    }
    return nullptr;
}

void Helper::addOutput()
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

void Helper::setOutputMode(OutputMode mode)
{
    if (m_outputList.length() < 2 || m_mode == mode)
        return;

    m_mode = mode;
    Q_EMIT outputModeChanged();
    for (int i = 0; i < m_outputList.size(); i++) {
        if (m_outputList.at(i) == m_surfaceContainer->primaryOutput())
            continue;

        Output *o = nullptr;
        if (mode == OutputMode::Extension) {
            o = Output::createPrimary(m_outputList.at(i)->output(), qmlEngine(), this);
            o->outputItem()->stackBefore(m_surfaceContainer);
            m_surfaceContainer->addOutput(o);
            enableOutput(o->output());
        } else { // Copy
            o = Output::createCopy(m_outputList.at(i)->output(),
                                   m_surfaceContainer->primaryOutput(),
                                   qmlEngine(),
                                   this);
            m_surfaceContainer->removeOutput(m_outputList.at(i));
        }

        m_outputList.at(i)->deleteLater();
        m_outputList.replace(i, o);
    }
}

Helper::OutputMode Helper::outputMode() const
{
    return m_mode;
}
