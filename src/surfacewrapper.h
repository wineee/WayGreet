// Copyright (C) 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#pragma once

#include <wsurfaceitem.h>
#include <wtoplevelsurface.h>

#include <QQuickItem>

Q_MOC_INCLUDE(<woutput.h>)

WAYLIB_SERVER_USE_NAMESPACE

class QmlEngine;
class Output;
class SurfaceContainer;
class SurfaceWrapper : public QQuickItem
{
    friend class Helper;
    friend class SurfaceContainer;
    friend class SurfaceProxy;
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("SurfaceWrapper objects are created by c++")
    Q_PROPERTY(Type type READ type CONSTANT)
    // make to readonly
    Q_PROPERTY(qreal implicitWidth READ implicitWidth NOTIFY implicitWidthChanged FINAL)
    Q_PROPERTY(qreal implicitHeight READ implicitHeight NOTIFY implicitHeightChanged FINAL)
    Q_PROPERTY(WAYLIB_SERVER_NAMESPACE::WSurface* surface READ surface CONSTANT)
    Q_PROPERTY(WAYLIB_SERVER_NAMESPACE::WToplevelSurface* shellSurface READ shellSurface CONSTANT)
    Q_PROPERTY(WAYLIB_SERVER_NAMESPACE::WSurfaceItem* surfaceItem READ surfaceItem CONSTANT)
    Q_PROPERTY(QRectF boundingRect READ boundingRect NOTIFY boundingRectChanged)
    Q_PROPERTY(QRectF geometry READ geometry NOTIFY geometryChanged FINAL)
    Q_PROPERTY(QRectF normalGeometry READ normalGeometry NOTIFY normalGeometryChanged FINAL)
    Q_PROPERTY(QRectF maximizedGeometry READ maximizedGeometry NOTIFY maximizedGeometryChanged FINAL)
    Q_PROPERTY(Output* ownsOutput READ ownsOutput NOTIFY ownsOutputChanged FINAL)
    Q_PROPERTY(bool positionAutomatic READ positionAutomatic WRITE setPositionAutomatic NOTIFY positionAutomaticChanged FINAL)
    Q_PROPERTY(State previousSurfaceState READ previousSurfaceState NOTIFY previousSurfaceStateChanged FINAL)
    Q_PROPERTY(State surfaceState READ surfaceState NOTIFY surfaceStateChanged BINDABLE bindableSurfaceState FINAL)
    Q_PROPERTY(SurfaceContainer* container READ container NOTIFY containerChanged FINAL)
    Q_PROPERTY(bool clipInOutput READ clipInOutput WRITE setClipInOutput NOTIFY clipInOutputChanged FINAL)

public:
    enum class Type {
        XdgToplevel,
        XdgPopup,
        Layer,
        InputPopup,
    };
    Q_ENUM(Type)

    enum class State {
        Normal,
        Maximized,
        Minimized,
        Fullscreen,
    };
    Q_ENUM(State)

    explicit SurfaceWrapper(QmlEngine *qmlEngine,
                            WToplevelSurface *shellSurface, Type type,
                            QQuickItem *parent = nullptr);
    ~SurfaceWrapper() override;

    void setFocus(bool focus, Qt::FocusReason reason);

    WSurface *surface() const;
    WToplevelSurface *shellSurface() const;
    WSurfaceItem *surfaceItem() const;
    bool resize(const QSizeF &size);

    QRectF titlebarGeometry() const;
    QRectF boundingRect() const override;

    Type type() const;
    SurfaceWrapper *parentSurface() const;

    Output *ownsOutput() const;
    void setOwnsOutput(Output *newOwnsOutput);
    void setOutputs(const QList<WOutput*> &outputs);

    QRectF geometry() const;
    QRectF normalGeometry() const;
    void moveNormalGeometryInOutput(const QPointF &position);

    QRectF maximizedGeometry() const;
    void setMaximizedGeometry(const QRectF &newMaximizedGeometry);

    QRectF fullscreenGeometry() const;
    void setFullscreenGeometry(const QRectF &newFullscreenGeometry);

    bool positionAutomatic() const;
    void setPositionAutomatic(bool newPositionAutomatic);

    void resetWidth();
    void resetHeight();

    State previousSurfaceState() const;
    State surfaceState() const;
    void setSurfaceState(State newSurfaceState);
    QBindable<State> bindableSurfaceState();
    bool isNormal() const;
    bool isMaximized() const;
    bool isMinimized() const;
    bool isAnimationRunning() const;

    void setRadius(qreal newRadius);

    SurfaceContainer *container() const;

    void addSubSurface(SurfaceWrapper *surface);
    void removeSubSurface(SurfaceWrapper *surface);
    const QList<SurfaceWrapper*> &subSurfaces() const;
    SurfaceWrapper *stackFirstSurface() const;
    SurfaceWrapper *stackLastSurface() const;
    bool hasChild(SurfaceWrapper *child) const;

    bool clipInOutput() const;
    void setClipInOutput(bool newClipInOutput);
    QRectF clipRect() const override;

public Q_SLOTS:
    // for titlebar
    void requestMinimize();
    void requestCancelMinimize();
    void requestMaximize();
    void requestCancelMaximize();
    void requestToggleMaximize();
    void requestFullscreen();
    void requestCancelFullscreen();
    void requestClose();

    bool stackBefore(QQuickItem *item);
    bool stackAfter(QQuickItem *item);
    void stackToLast();

Q_SIGNALS:
    void boundingRectChanged();
    void ownsOutputChanged();
    void normalGeometryChanged();
    void maximizedGeometryChanged();
    void positionAutomaticChanged();
    void previousSurfaceStateChanged();
    void surfaceStateChanged();
    void requestMove(); // for titlebar
    void requestResize(Qt::Edges edges);
    void geometryChanged();
    void containerChanged();
    void clipInOutputChanged();

private:
    using QQuickItem::setParentItem;
    using QQuickItem::stackBefore;
    using QQuickItem::stackAfter;
    void setParent(QQuickItem *item);
    void setActivate(bool activate);
    void setNormalGeometry(const QRectF &newNormalGeometry);
    void setBoundedRect(const QRectF &newBoundedRect);
    void setContainer(SurfaceContainer *newContainer);
    void updateBoundingRect();
    void updateVisible();
    void updateSubSurfaceStacking();
    void updateClipRect();
    void geometryChange(const QRectF &newGeo, const QRectF &oldGeometry) override;

    void doSetSurfaceState(State newSurfaceState);
    bool startStateChangeAnimation(SurfaceWrapper::State targetState, const QRectF &targetGeometry);

    QmlEngine *m_engine;
    QPointer<SurfaceContainer> m_container;
    QList<SurfaceWrapper*> m_subSurfaces;
    SurfaceWrapper *m_parentSurface = nullptr;

    WToplevelSurface *m_shellSurface = nullptr;
    WSurfaceItem *m_surfaceItem = nullptr;
    QPointer<QQuickItem> m_titleBar;
    QPointer<QQuickItem> m_decoration;
    QPointer<QQuickItem> m_geometryAnimation;
    QRectF m_boundedRect;
    QRectF m_normalGeometry;
    QRectF m_maximizedGeometry;
    QRectF m_fullscreenGeometry;
    Type m_type;
    QPointer<Output> m_ownsOutput;
    QPointF m_positionInOwnsOutput;
    SurfaceWrapper::State m_pendingState;
    QRectF m_pendingGeometry;
    Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(SurfaceWrapper, SurfaceWrapper::State, m_previousSurfaceState, State::Normal, &SurfaceWrapper::previousSurfaceStateChanged)
    Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(SurfaceWrapper, SurfaceWrapper::State, m_surfaceState, State::Normal, &SurfaceWrapper::surfaceStateChanged)

    struct TitleBarState {
        constexpr static uint Default = 0;
        constexpr static uint Visible = 1;
        constexpr static uint Hidden = 2;
    };

    uint m_positionAutomatic:1;
    uint m_clipInOutput:1;
};
