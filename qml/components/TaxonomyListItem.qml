import QtQuick 2.0
import Sailfish.Silica 1.0

ListItem {
    property string name
    property int count
    property string iconSource: "image://theme/icon-m-folder"

    id: listItem
    contentHeight: Theme.itemSizeMedium

    Image {
        id: icon
        anchors {
            left: parent.left
            leftMargin: Theme.horizontalPageMargin
            verticalCenter: parent.verticalCenter
        }
        source: iconSource + "?" + (highlighted ? Theme.highlightColor : Theme.primaryColor)
    }

    Label {
        id: nameLabel
        anchors {
            left: icon.right
            right: countLabel.left
            verticalCenter: parent.verticalCenter
            leftMargin: Theme.paddingMedium
            rightMargin: Theme.paddingMedium
        }
        color: highlighted ? Theme.highlightColor : Theme.primaryColor
        truncationMode: TruncationMode.Fade
        text: name
    }

    Label {
        id: countLabel
        anchors {
            right: parent.right
            rightMargin: Theme.horizontalPageMargin
            verticalCenter: parent.verticalCenter
        }
        color: highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
        font.pixelSize: Theme.fontSizeSmall
        text: count
    }
}
