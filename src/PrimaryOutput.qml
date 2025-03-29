// Copyright (C) 2025 rewine <luhongxu@deepin.org>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import Waylib.Server
import WayGreet

OutputItem {
    id: rootOutputItem
    readonly property OutputViewport screenViewport: outputViewport
    property bool forceSoftwareCursor: false

    devicePixelRatio: output?.scale ?? devicePixelRatio

    cursorDelegate: Cursor {
        id: cursorItem

        required property QtObject outputCursor
        readonly property point position: parent.mapFromGlobal(cursor.position.x, cursor.position.y)

        cursor: outputCursor.cursor
        output: outputCursor.output.output
        x: position.x - hotSpot.x
        y: position.y - hotSpot.y
        visible: valid && outputCursor.visible
        OutputLayer.enabled: !outputCursor.output.forceSoftwareCursor
        OutputLayer.keepLayer: true
        OutputLayer.outputs: [screenViewport]
        OutputLayer.flags: OutputLayer.Cursor
        OutputLayer.cursorHotSpot: hotSpot

        themeName: WayConfig.cursorTheme
        sourceSize: WayConfig.cursorSize
    }

    OutputViewport {
        id: outputViewport

        output: rootOutputItem.output
        devicePixelRatio: parent.devicePixelRatio
        anchors.centerIn: parent

        RotationAnimation {
            id: rotationAnimator

            target: outputViewport
            duration: 200
            alwaysRunToEnd: true
        }

        Timer {
            id: setTransform

            property var scheduleTransform
            onTriggered: screenViewport.rotateOutput(scheduleTransform)
            interval: rotationAnimator.duration / 2
        }

        function rotationOutput(orientation) {
            setTransform.scheduleTransform = orientation
            setTransform.start()

            switch(orientation) {
            case WaylandOutput.R90:
                rotationAnimator.to = 90
                break
            case WaylandOutput.R180:
                rotationAnimator.to = 180
                break
            case WaylandOutput.R270:
                rotationAnimator.to = -90
                break
            default:
                rotationAnimator.to = 0
                break
            }

            rotationAnimator.from = rotation
            rotationAnimator.start()
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "#197e4a"
    }

    Greeter {
        anchors.fill: parent
    }

    function setTransform(transform) {
        screenViewport.rotationOutput(transform)
    }

    function setScale(scale) {
        screenViewport.setOutputScale(scale)
    }

    function invalidate() {
        screenViewport.invalidate()
    }
}
