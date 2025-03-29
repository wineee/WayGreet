// Copyright (C) 2025 rewine <luhongxu@deepin.org>
// SPDX-License-Identifier: GPL-3.0-or-later

// Fork from https://gitlab.com/isseigx/simplicity-sddm-theme

import QtQuick
import QtQuick.Controls

Button {
    id: control

    contentItem: Text {
        text: control.text
        font: control.font
        color: "white"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        implicitWidth: 100
        implicitHeight: 30
        color: control.down ? Qt.rgba(0, 0, 0, 0.6) : Qt.rgba(0, 0, 0, 0.4)
        border.color: Qt.rgba(1, 1, 1, 0.4)
        border.width: 1
        radius: 3
    }
}
