import QtQuick 2.0
import Sailfish.Silica 1.0

import "../components"

StandardDialog {
    property alias username: usernameField.text
    property alias password: passwordField.text

    id: page

    //% "Log In"
    acceptText: qsTrId("login.login")
    //% "Cancel"
    cancelText: qsTrId("dialog.cancel")

    canAccept: username && password

    TextField {
        id: usernameField
        //% "Username"
        label: qsTrId("login.username")

        EnterKey.iconSource: "image://theme/icon-m-enter-next"
        EnterKey.onClicked: passwordField.focus = true;
    }

    TextField {
        property bool passwordVisible: false

        id: passwordField
        //% "Password"
        label: qsTrId("login.password")
        echoMode: passwordVisible ? TextInput.Normal : TextInput.Password
        rightItem: IconButton {
            icon.source: !passwordField.passwordVisible
                         ? "image://theme/icon-splus-hide-password"
                         : "image://theme/icon-splus-show-password"
            onClicked: {
                passwordField.passwordVisible = !passwordField.passwordVisible;
            }
        }
        EnterKey.iconSource: "image://theme/icon-m-enter-accept"
        EnterKey.onClicked: page.accept();
    }
}
