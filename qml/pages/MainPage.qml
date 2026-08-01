import QtQuick 2.0
import Sailfish.Silica 1.0

import "../components"

StandardPage {
    //% "Recipes"
    title: qsTrId("main.title")

    PullDownMenu {
        MenuItem {
            //% "Logout"
            text: qsTrId("main.logout")
            onClicked: {
                secrets.password = '';
                core.reinitialize();
                safeCall(function() {
                    pageStack.replace("CheckPage.qml");
                });
            }
        }
    }
}
