import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import WayGreet

Item {
    id: root
    anchors.fill: parent

    // Handle authentication messages
    Connections {
        target: Helper
        function onSessionSuccess() { }
        function onSessionError(type, description) {
            errorLabel.text = "Login Failed: " + description
            errorLabel.visible = true
            passwordField.clear()
            passwordField.focus = true
        }
    }

    // A simple black background for minimal theme
    Rectangle {
        anchors.fill: parent
        color: "#111111"
    }

    // Centered login box
    ColumnLayout {
        anchors.centerIn: parent
        spacing: 15

        Label {
            text: "WayGreet Minimal Theme"
            font.pixelSize: 24
            color: "white"
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 20
        }

        ComboBox {
            id: userComboBox
            model: Helper.userModel
            textRole: "realName"
            valueRole: "name"
            currentIndex: Helper.userModel.lastIndex
            Layout.fillWidth: true
        }

        ComboBox {
            id: sessionComboBox
            model: Helper.sessionModel
            textRole: "name"
            currentIndex: Helper.sessionModel.lastIndex
            Layout.fillWidth: true
            visible: Helper.sessionModel.rowCount() > 0
            onCurrentIndexChanged: {
                Helper.sessionModel.setLastIndex(currentIndex)
            }
        }

        TextField {
            id: passwordField
            echoMode: TextInput.Password
            placeholderText: "Password"
            focus: true
            Layout.fillWidth: true
            onAccepted: loginButton.clicked()
        }

        Button {
            id: loginButton
            text: "Login"
            Layout.fillWidth: true
            onClicked: Helper.login(userComboBox.currentValue, passwordField.text, sessionComboBox.currentIndex)
        }

        Label {
            id: errorLabel
            color: "#ff5555"
            visible: false
            Layout.alignment: Qt.AlignHCenter
        }
    }

    // Power buttons at the bottom
    RowLayout {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 20
        spacing: 10

        Button {
            text: "Sleep"
            visible: PowerManager.canSuspend()
            onClicked: PowerManager.suspend()
        }
        Button {
            text: "Reboot"
            visible: PowerManager.canReboot()
            onClicked: PowerManager.reboot()
        }
        Button {
            text: "Power Off"
            visible: PowerManager.canPowerOff()
            onClicked: PowerManager.powerOff()
        }
    }
}
