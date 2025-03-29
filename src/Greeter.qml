import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import WayGreet

Item {
    id: root

    readonly property color backgroundColor: Qt.rgba(0, 0, 0, 0.4)
    readonly property color hoverBackgroundColor: Qt.rgba(0, 0, 0, 0.6)

    LayoutMirroring.enabled: Qt.locale().textDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    Connections {
        target: Greetd

        function onSessionSuccess() {
        }
        function onSessionError(type, description) {
            if (type === "auth_error") {
                pw_entry.clear()
                pw_entry.focus = true

                errorMsgContainer.visible = true
            } else {
                console.log(type, description);
            }
        }
        function onInfoMessage(message) {
            console.log(message);
        }
        function onErrorMessage(message) {
            console.log(message);
        }
    }

    Image {
        anchors.fill: parent
        source: WayConfig.background
    }

    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        spacing: 10

        SimpleComboBox {
            id: user_entry
            model: Helper.userModel
            currentIndex: Helper.userModel.lastIndex
            textRole: "realName"
            width: 250
            KeyNavigation.backtab: session
            KeyNavigation.tab: pw_entry
        }

        TextField {
            id: pw_entry
            color: "white"
            echoMode: TextInput.Password
            focus: true
            placeholderText: "Enter your password"
            placeholderTextColor: Qt.rgba(1, 1, 1, 0.4)
            width: 250
            background: Rectangle {
                implicitWidth: 100
                implicitHeight: 30
                color: pw_entry.activeFocus ? hoverBackgroundColor : backgroundColor
                border.color: Qt.rgba(1, 1, 1, 0.4)
                radius: 3
            }
            onAccepted: Greetd.login(user_entry.getValue(), pw_entry.text) // session.currentIndex
            KeyNavigation.backtab: user_entry
            KeyNavigation.tab: loginButton
        }

        SimpleButton {
            id: loginButton
            text: "login"
            width: 250
            onClicked: Greetd.login(user_entry.getValue(), pw_entry.text) // session.currentIndex
            KeyNavigation.backtab: pw_entry
            KeyNavigation.tab: suspend
        }

        Rectangle {
            id: errorMsgContainer
            width: 250
            height: loginButton.height
            color: "#F44336"
            clip: true
            visible: false
            radius: 3

            Label {
                anchors.centerIn: parent
                text: "Login failed"
                width: 240
                color: "white"
                font.bold: true
                elide: Qt.locale().textDirection === Qt.RightToLeft ? Text.ElideLeft : Text.ElideRight
                horizontalAlignment: Qt.AlignHCenter
            }
        }

    }

    Row {
        anchors {
            bottom: parent.bottom
            bottomMargin: 10
            horizontalCenter: parent.horizontalCenter
        }

        spacing: 5

        SimpleButton {
            id: suspend
            text: "suspend"
            onClicked: WayPowerManager.suspend()
            //visible: sddm.canSuspend
            KeyNavigation.backtab: loginButton
            KeyNavigation.tab: hibernate
        }

        SimpleButton {
            id: hibernate
            text: "hibernate"
            onClicked: WayPowerManager.hibernate()
            //visible: sddm.canHibernate
            KeyNavigation.backtab: suspend
            KeyNavigation.tab: restart
        }

        SimpleButton {
            id: restart
            text: "reboot"
            onClicked: WayPowerManager.reboot()
            //visible: sddm.canReboot
            KeyNavigation.backtab: suspend
            KeyNavigation.tab: shutdown
        }

        SimpleButton {
            id: shutdown
            text: "shutdown"
            onClicked: WayPowerManager.powerOff()
            //visible: sddm.canPowerOff
            KeyNavigation.backtab: restart
            KeyNavigation.tab: session
        }
    }

    SimpleComboBox {
        id: session
        anchors {
            left: parent.left
            leftMargin: 10
            top: parent.top
            topMargin: 10
        }
        currentIndex: Helper.sessionModel.lastIndex
        model: Helper.sessionModel
        textRole: "name"
        width: 200
        visible: Helper.sessionModel.rowCount() > 1
        KeyNavigation.backtab: shutdown
        KeyNavigation.tab: user_entry
        onCurrentIndexChanged: {
            Helper.sessionModel.setLastIndex(currentIndex)
        }
    }

    Rectangle {
        id: timeContainer
        anchors {
            top: parent.top
            right: parent.right
            topMargin: 10
            rightMargin: 10
        }
        border.color: Qt.rgba(1, 1, 1, 0.4)
        radius: 3
        color: backgroundColor
        width: timelb.width + 10
        height: session.height

        Label {
            id: timelb
            anchors.centerIn: parent
            text: Qt.formatDateTime(new Date(), "HH:mm")
            color: "white"
            horizontalAlignment: Text.AlignHCenter
        }
    }

    Timer {
        id: timetr
        interval: 500
        repeat: true
        running: true
        onTriggered: {
            timelb.text = Qt.formatDateTime(new Date(), "HH:mm")
        }
    }
}
