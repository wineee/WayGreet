// Copyright (C) 2025 rewine <luhongxu@deepin.org>
// SPDX-License-Identifier: GPL-3.0-or-later

// Fork from https://gitlab.com/isseigx/simplicity-sddm-theme

import QtQuick
import QtQuick.Controls

ComboBox {
    id: control

    delegate: ItemDelegate {
        id: itemDelegate
        text: model.realName ? model.realName : model.name
        width: control.width
        contentItem: Text {
            text: itemDelegate.text
            color: "white"
            font: itemDelegate.font
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }
        background: Rectangle {
            visible: itemDelegate.down || itemDelegate.highlighted || itemDelegate.visualFocus
            color: Qt.rgba(0, 0, 0, 0.6)
        }
        highlighted: control.highlightedIndex === index
    }

    indicator: Canvas {
        id: canvas
        x: control.width - width - control.rightPadding
        y: control.topPadding + (control.availableHeight - height) / 2
        width: 12
        height: 8
        contextType: "2d"

        Connections {
            target: control
            function onPressedChanged() { canvas.requestPaint(); }
        }

        onPaint: {
            context.reset();
            context.moveTo(0, 0);
            context.lineTo(width, 0);
            context.lineTo(width / 2, height);
            context.closePath();
            context.fillStyle = "white";
            context.fill();
        }
    }

    contentItem: Text {
        leftPadding: 5
        rightPadding: control.indicator.width + control.spacing

        text: control.displayText ? control.displayText : getValue()
        font: control.font
        color: "white"
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        implicitWidth: 120
        implicitHeight: 30
        border.color: Qt.rgba(1, 1, 1, 0.4)
        border.width: 1
        color: control.pressed ? Qt.rgba(0, 0, 0, 0.6) : Qt.rgba(0, 0, 0, 0.4)
        radius: 3
    }

    popup: Popup {
        y: control.height - 1
        width: control.width
        implicitHeight: contentItem.implicitHeight
        padding: 1

        height: Math.min(contentItem.implicitHeight, control.Window.height - topMargin - bottomMargin)

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
            ScrollIndicator.vertical: ScrollIndicator { }
        }

        background: Rectangle {
            border.color: Qt.rgba(1, 1, 1, 0.4)
            color: Qt.rgba(0, 0, 0, 0.5)
        }
    }

    function getValue() {
        var items = control.delegateModel.items
        var index = control.currentIndex
        if (0 <= index && index < items.count) {
            return items.get(index).model.name
        }
        // index error, return last user
        else {
            return userModel.lastUser
        }
    }
}
