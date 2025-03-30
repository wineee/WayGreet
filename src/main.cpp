// Copyright (C) 2025 rewine <luhongxu@deepin.org>.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "helper.h"
#include "wayconfig.h"
#include "powermanager.h"

#include <wrenderhelper.h>

#include <qwbuffer.h>
#include <qwlogging.h>

#include <QGuiApplication>

WAYLIB_SERVER_USE_NAMESPACE

int main(int argc, char *argv[])
{
    qw_log::init(WLR_ERROR);

    WRenderHelper::setupRendererBackend();
    Q_ASSERT(qw_buffer::get_objects().isEmpty());

    WServer::initializeQPA();

    QPointer<Helper> helper;
    int quitCode = 0;
    {
        QGuiApplication::setAttribute(Qt::AA_UseOpenGLES);
        QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
            Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
        QGuiApplication::setQuitOnLastWindowClosed(false);
        QGuiApplication app(argc, argv);

        QmlEngine qmlEngine;

        QObject::connect(&qmlEngine, &QQmlEngine::quit, &app, &QGuiApplication::quit);
        QObject::connect(&qmlEngine, &QQmlEngine::exit, &app, [](int code) {
            qApp->exit(code);
        });

        auto config = qmlEngine.singletonInstance<WayConfig *>("WayGreet", "WayConfig");
        Q_ASSERT(config);

        auto helper = qmlEngine.singletonInstance<Helper *>("WayGreet", "Helper");
        helper->init();

        auto powermanager = qmlEngine.singletonInstance<PowerManager *>("WayGreet", "PowerManager");
        Q_ASSERT(powermanager);

        quitCode = app.exec();
    }

    Q_ASSERT(!helper);
    Q_ASSERT(qw_buffer::get_objects().isEmpty());

    return quitCode;
}
