import QtQuick 2.0
import Sailfish.Silica 1.0

import "../components"

StandardPage {
    property var categories: []
    property string errorText

    id: page
    //% "Categories"
    title: qsTrId("categories.title")

    function refresh() {
        errorText = "";
        loading = true;
        //% "Loading categories..."
        loadText = qsTrId("categories.loading");
        core.listCategories();
    }

    function displayCategoryName(category) {
        if (category.name === "*") {
            //% "Uncategorized"
            return qsTrId("categories.uncategorized");
        }

        return category.name || "";
    }

    Connections {
        target: core

        onCategoriesResolved: {
            loading = false;
            if (!success) {
                //% "Could not load categories. Pull down to refresh."
                errorText = qsTrId("categories.error");
                return;
            }

            page.categories = categories;
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
        //% "No categories"
        text: qsTrId("categories.empty")
        color: Theme.secondaryColor
        visible: !errorText && categories.length === 0
        horizontalAlignment: Text.AlignHCenter
    }

    Repeater {
        model: categories

        TaxonomyListItem {
            width: page.width
            name: displayCategoryName(modelData)
            count: modelData.recipeCount || 0

            onClicked: console.warn("Category selection is not implemented yet")
        }
    }

    Component.onCompleted: refresh()
}
