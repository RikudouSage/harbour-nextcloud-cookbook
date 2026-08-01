import QtQuick 2.0
import Sailfish.Silica 1.0

import "../components"

StandardPage {
    property var tags: []
    property string errorText

    id: page
    //% "Tags"
    title: qsTrId("tags.title")

    function refresh() {
        errorText = "";
        loading = true;
        //% "Loading tags..."
        loadText = qsTrId("tags.loading");
        core.listKeywords();
    }

    Connections {
        target: core

        onKeywordsResolved: {
            loading = false;
            if (!success) {
                //% "Could not load tags. Pull down to refresh."
                errorText = qsTrId("tags.error");
                return;
            }

            page.tags = keywords;
        }
    }

    PullDownMenu {
        MenuItem {
            //% "Refresh"
            text: qsTrId("main.refresh")
            onClicked: refresh()
        }
    }

    StandardLabel {
        text: errorText
        color: Theme.errorColor
        visible: text.length > 0
    }

    StandardLabel {
        //% "No tags"
        text: qsTrId("tags.empty")
        color: Theme.secondaryColor
        visible: !errorText && tags.length === 0
        horizontalAlignment: Text.AlignHCenter
    }

    Repeater {
        model: tags

        TaxonomyListItem {
            width: page.width
            name: modelData.name || ""
            count: modelData.recipeCount || 0
            iconSource: "image://theme/icon-m-levels"

            onClicked: pageStack.push("TagDetailPage.qml", {
                tagName: modelData.name || ""
            })
        }
    }

    Component.onCompleted: refresh()
}
