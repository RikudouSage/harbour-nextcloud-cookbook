import QtQuick 2.0
import Sailfish.Silica 1.0

import "../components"

StandardDialog {
    property alias url: urlField.text

    id: page
    //% "Import from URL"
    acceptText: qsTrId("main.import_from_url")
    //% "Cancel"
    cancelText: qsTrId("dialog.cancel")
    canAccept: urlField.text.trim().length > 0

    TextField {
        id: urlField
        //% "Recipe URL"
        label: qsTrId("import_recipe.url")
        inputMethodHints: Qt.ImhUrlCharactersOnly

        EnterKey.iconSource: "image://theme/icon-m-enter-accept"
        EnterKey.onClicked: page.accept()
    }
}
