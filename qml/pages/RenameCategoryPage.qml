import QtQuick 2.0
import Sailfish.Silica 1.0

import "../components"

StandardDialog {
    property alias categoryName: nameField.text

    id: page
    //% "Rename category"
    acceptText: qsTrId("category_detail.rename")
    //% "Cancel"
    cancelText: qsTrId("dialog.cancel")
    canAccept: nameField.text.trim().length > 0

    TextField {
        id: nameField
        //% "Category name"
        label: qsTrId("category_detail.category_name")

        EnterKey.iconSource: "image://theme/icon-m-enter-accept"
        EnterKey.onClicked: page.accept()
    }
}
