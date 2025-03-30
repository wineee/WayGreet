// Copyright (C) 2024 UnionTech Software Technology Co., Ltd.
// Copyright (C) 2025 rewine <luhongxu@deepin.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "rootcontainer.h"

#include "helper.h"
#include "output.h"

#include <wcursor.h>
#include <woutput.h>
#include <woutputitem.h>
#include <woutputlayout.h>

#include <qwoutputlayout.h>

#include <QQuickWindow>

WAYLIB_SERVER_USE_NAMESPACE

RootContainer::RootContainer(QQuickItem *parent)
    : QQuickItem(parent)
    , m_cursor(new WCursor(this))
{
    m_cursor->setEventWindow(window());
}

void RootContainer::init(WServer *server)
{
    m_outputLayout = new WOutputLayout(server);
    m_cursor->setLayout(m_outputLayout);

    connect(m_outputLayout, &WOutputLayout::implicitWidthChanged, this, [this] {
        const auto width = m_outputLayout->implicitWidth();
        window()->setWidth(width);
        setWidth(width);
    });

    connect(m_outputLayout, &WOutputLayout::implicitHeightChanged, this, [this] {
        const auto height = m_outputLayout->implicitHeight();
        window()->setHeight(height);
        setHeight(height);
    });

    m_outputLayout->safeConnect(&qw_output_layout::notify_change, this, [this] {
        for (auto output : std::as_const(outputs())) {
            output->updatePositionFromLayout();
        }

        ensureCursorVisible();
    });
}

WOutputLayout *RootContainer::outputLayout() const
{
    return m_outputLayout;
}

WCursor *RootContainer::cursor() const
{
    return m_cursor;
}

Output *RootContainer::cursorOutput() const
{
    Q_ASSERT(m_cursor->layout() == m_outputLayout);
    const auto &pos = m_cursor->position();
    auto o = m_outputLayout->handle()->output_at(pos.x(), pos.y());
    if (!o)
        return nullptr;

    return getOutput(WOutput::fromHandle(qw_output::from(o)));
}

Output *RootContainer::primaryOutput() const
{
    return m_primaryOutput;
}

void RootContainer::setPrimaryOutput(Output *newPrimaryOutput)
{
    if (m_primaryOutput == newPrimaryOutput)
        return;
    m_primaryOutput = newPrimaryOutput;
    emit primaryOutputChanged();
}

const QList<Output *> &RootContainer::outputs() const
{
    return m_outputList;
}

void RootContainer::ensureCursorVisible()
{
    const auto cursorPos = m_cursor->position();
    if (m_outputLayout->handle()->output_at(cursorPos.x(), cursorPos.y()))
        return;

    if (m_primaryOutput) {
        Helper::instance()->setCursorPosition(m_primaryOutput->geometry().center());
    }
}

void RootContainer::allowNonDrmOutputAutoChangeMode(WOutput *output)
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

void RootContainer::enableOutput(WOutput *output)
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

int RootContainer::indexOfOutput(WOutput *output) const
{
    for (int i = 0; i < m_outputList.size(); i++) {
        if (m_outputList.at(i)->output() == output)
            return i;
    }
    return -1;
}

Output *RootContainer::getOutput(WOutput *output) const
{
    for (auto o : std::as_const(m_outputList)) {
        if (o->output() == output)
            return o;
    }
    return nullptr;
}

void RootContainer::setOutputMode(OutputMode mode)
{
    if (m_outputList.length() < 2 || m_mode == mode)
        return;

    m_mode = mode;
    for (int i = 0; i < m_outputList.size(); i++) {
        if (m_outputList.at(i) == primaryOutput())
            continue;

        Output *o = nullptr;
        if (mode == OutputMode::Extension) {
            o = Output::createPrimary(m_outputList.at(i)->output(),
                                      Helper::instance()->qmlEngine(),
                                      this);
            o->outputItem()->stackBefore(this);
            addOutput(o);
            enableOutput(o->output());
        } else { // Copy
            o = Output::createCopy(m_outputList.at(i)->output(),
                                   primaryOutput(),
                                   Helper::instance()->qmlEngine(),
                                   this);
            removeOutput(o);
        }

        m_outputList.at(i)->deleteLater();
        m_outputList.replace(i, o);
    }
}

RootContainer::OutputMode RootContainer::outputMode() const
{
    return m_mode;
}

void RootContainer::onOutputAdded(WOutput *output)
{
    allowNonDrmOutputAutoChangeMode(output);
    Output *o;
    if (m_mode == OutputMode::Extension || !primaryOutput()) {
        o = Output::createPrimary(output, Helper::instance()->qmlEngine(), this);
        o->outputItem()->stackBefore(this);
        addOutput(o);
    } else if (m_mode == OutputMode::Copy) {
        o = Output::createCopy(output, primaryOutput(), Helper::instance()->qmlEngine(), this);
    }

    enableOutput(output);
}

void RootContainer::onOutputRemoved(WOutput *output)
{
    auto index = indexOfOutput(output);
    Q_ASSERT(index >= 0);
    const auto o = m_outputList.takeAt(index);
    removeOutput(o);
    delete o;
    if (outputs().isEmpty() && Helper::instance()->isTestMode())
        qApp->quit();
}

void RootContainer::addOutput(Output *output)
{
    m_outputList.append(output);
    m_outputLayout->autoAdd(output->output());
    if (!m_primaryOutput)
        setPrimaryOutput(output);
}

void RootContainer::removeOutput(Output *output)
{
    m_outputLayout->remove(output->output());
    if (m_primaryOutput == output) {
        const auto outputs = m_outputLayout->outputs();
        if (!outputs.isEmpty()) {
            auto newPrimaryOutput = getOutput(outputs.first());
            setPrimaryOutput(newPrimaryOutput);
        }
    }

    // ensure cursor within output
    const auto outputPos = output->outputItem()->position();
    if (output->geometry().contains(m_cursor->position()) && m_primaryOutput) {
        const auto posInOutput = m_cursor->position() - outputPos;
        const auto newCursorPos = m_primaryOutput->outputItem()->position() + posInOutput;

        if (m_primaryOutput->geometry().contains(newCursorPos))
            Helper::instance()->setCursorPosition(newCursorPos);
        else
            Helper::instance()->setCursorPosition(m_primaryOutput->geometry().center());
    }
    m_outputList.removeOne(output);
}
