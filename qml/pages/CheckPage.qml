import QtQuick 2.0
import Sailfish.Silica 1.0

import "../components"

StandardPage {
    //% "Checking..."
    title: qsTrId("check_page.title")
    loading: true

    function pushMain() {
        safeCall(function() {
            pageStack.replace("MainPage.qml")
        });
    }

    function pushLogin(errorText) {
        safeCall(function() {
            const dialog = pageStack.push("LoginPage.qml", {
                username: secrets.username,
                nextcloudUrl: secrets.nextcloudUrl,
                errorText: errorText,
            });
            dialog.rejected.connect(function() {
                Qt.quit();
            });

            dialog.accepted.connect(function() {
                secrets.nextcloudUrl = dialog.nextcloudUrl;
                secrets.username = dialog.username;

                core.validateCredentials(
                    dialog.nextcloudUrl,
                    dialog.username,
                    dialog.password
                );
                loading = true;
                //% "Validating credentials..."
                loadText = qsTrId("check.validating_credentials");
            });
        });
    }

    StandardLabel {
        id: errorLabel
        color: Theme.errorColor
        visible: text.length > 0
    }


    Connections {
        target: core

        onCredentialsValidated: {
            if (!success) {
                //% "Invalid username or password"
                pushLogin(qsTrId("check.invalid_credentials"))
                return;
            }

            secrets.nextcloudUrl = url;
            secrets.username = username;
            secrets.password = password;

            core.reinitialize();
            //% "Initializing..."
            loadText = qsTrId("check.initializing")
        }

        onInitialized: {
            loading = false;
            if (!success) {
                //% "There was an internal error while initializing. There's nothing you can do except reporting it to the developer."
                errorLabel.text = qsTrId("check.failed_initializing")
                return;
            }

            pushMain();
        }
    }

    Component.onCompleted: {
        if (secrets.username && secrets.password && secrets.nextcloudUrl) {
            pushMain();
        } else {
            pushLogin();
        }
    }
}
