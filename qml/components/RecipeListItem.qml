import QtQuick 2.0
import Sailfish.Silica 1.0

ListItem {
    property var recipe
    property string imageSource
    property bool imageRequested: false
    property bool imageRetryRequested: false
    property bool completed: false

    signal editRequested(var recipe)
    signal deleteRequested(var recipe)
    signal shareRequested(var recipe)

    id: listItem
    contentHeight: Math.max(Theme.itemSizeLarge, thumbnail.height + Theme.paddingMedium)
    menu: contextMenu

    function formatDate(value) {
        if (!value) {
            return "";
        }

        const date = new Date(value);
        return Format.formatDate(date, Format.DateMedium);
    }

    function remove() {
        remorseDelete(function() {
            listItem.deleteRequested(recipe);
            listItem.visible = false;
        });
    }

    function loadImage() {
        if (imageRequested || !recipe || !recipe.id) {
            return;
        }

        imageRequested = true;
        core.resolveRecipeImage(recipe.id);
    }

    onRecipeChanged: {
        imageSource = "";
        imageRequested = false;
        imageRetryRequested = false;

        if (completed) {
            deferredLoad.restart();
        }
    }

    Connections {
        target: core

        onRecipeImageResolved: {
            if (!recipe || id !== recipe.id) {
                return;
            }

            if (success) {
                imageSource = path;
            } else {
                console.warn("Recipe image resolving failed", id);
            }
        }
    }

    Rectangle {
        id: thumbnail
        width: Theme.itemSizeLarge
        height: Theme.itemSizeLarge
        radius: Theme.paddingSmall
        color: Theme.rgba(Theme.highlightBackgroundColor, highlighted ? 0.2 : 0.1)
        anchors {
            left: parent.left
            leftMargin: Theme.horizontalPageMargin
            verticalCenter: parent.verticalCenter
        }

        Image {
            id: recipeImage
            anchors.fill: parent
            fillMode: Image.PreserveAspectCrop
            source: imageSource
            visible: status === Image.Ready

            onStatusChanged: {
                if (status !== Image.Error || imageRetryRequested || !recipe || !recipe.id) {
                    return;
                }

                imageRetryRequested = true;
                imageSource = "";
                imageRequested = false;
                core.invalidateRecipeImage(recipe.id);
                loadImage();
            }
        }

        Image {
            anchors.centerIn: parent
            source: "image://theme/icon-m-file-image?" + Theme.secondaryColor
            visible: !recipeImage.visible
        }
    }

    Label {
        id: titleLabel
        anchors {
            left: thumbnail.right
            right: parent.right
            top: thumbnail.top
            leftMargin: Theme.paddingMedium
            rightMargin: Theme.horizontalPageMargin
        }
        color: highlighted ? Theme.highlightColor : Theme.primaryColor
        truncationMode: TruncationMode.Fade
        text: recipe.name || ""
    }

    Label {
        id: detailLabel
        anchors {
            left: titleLabel.left
            right: titleLabel.right
            top: titleLabel.bottom
            topMargin: Theme.paddingSmall
        }
        color: highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
        font.pixelSize: Theme.fontSizeSmall
        truncationMode: TruncationMode.Fade
        text: (recipe.keywords || []).join(", ")
        visible: text.length > 0
    }

    Label {
        anchors {
            left: titleLabel.left
            right: titleLabel.right
            bottom: thumbnail.bottom
        }
        color: highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
        font.pixelSize: Theme.fontSizeExtraSmall
        truncationMode: TruncationMode.Fade
        text: formatDate(recipe.modifiedDate || recipe.createdDate)
        visible: text.length > 0
    }

    Component {
        id: contextMenu

        ContextMenu {
//            MenuItem {
//                //% "Edit"
//                text: qsTrId("recipe_item.edit")
//                onClicked: listItem.editRequested(recipe)
//            }

            MenuItem {
                //% "Delete"
                text: qsTrId("recipe_item.delete")
                onClicked: listItem.remove()
            }

            MenuItem {
                //% "Share"
                text: qsTrId("recipe_item.share")
                onClicked: listItem.shareRequested(recipe)
            }
        }
    }

    Timer {
        id: deferredLoad
        interval: 1
        repeat: false
        onTriggered: loadImage()
    }

    Component.onCompleted: {
        completed = true;
        deferredLoad.restart();
    }
}
