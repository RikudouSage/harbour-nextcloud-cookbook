import QtQuick 2.0

import "../components"

StandardPage {
    //% "Checking..."
    title: qsTrId("check_page.title")

    Component.onCompleted: {
        if (secrets.username && secrets.password) {
        } else {
            safeCall(function() {
                const dialog = pageStack.push("LoginPage.qml");
                dialog.rejected.connect(function() {
                    Qt.quit();
                });

                dialog.accepted.connect(function() {
                });
            });
        }
    }
}
