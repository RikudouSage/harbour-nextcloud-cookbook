import QtQuick 2.0
import Sailfish.Silica 1.0

import "../components"

StandardPage {
    //% "Recipes"
    property string scopeTitle: qsTrId("main.title")
    property var recipes: []
    property string errorText

    id: page
    title: scopeTitle

    function refresh() {
        errorText = "";
        loading = true;
        //% "Loading recipes..."
        loadText = qsTrId("main.loading_recipes");

        if (search.text.length > 0) {
            core.searchRecipes(search.text);
        } else {
            core.listRecipes();
        }
    }

    function logout() {
        secrets.password = "";
        core.reinitialize();
        safeCall(function() {
            pageStack.replace("CheckPage.qml");
        });
    }

    Connections {
        target: core

        onRecipesResolved: {
            loading = false;
            if (!success) {
                //% "Could not load recipes. Pull down to refresh."
                errorText = qsTrId("main.recipes_error");
                return;
            }

            page.recipes = recipes;
        }

        onRecipeImported: {
            if (!success) {
                loading = false;
                //% "Could not import recipe."
                errorText = qsTrId("main.import_error");
                return;
            }

            refresh();
        }

        onRecipeDeleted: {
            if (!success) {
                //% "Could not delete recipe."
                errorText = qsTrId("main.delete_error");
                return;
            }

            refresh();
        }
    }

    PullDownMenu {
        MenuItem {
            //% "Logout"
            text: qsTrId("main.logout")
            onClicked: logout()
        }

        MenuItem {
            //% "Create recipe"
            text: qsTrId("main.create_recipe")
            onClicked: console.warn("Create recipe page is not implemented yet")
        }

        MenuItem {
            //% "Import from URL"
            text: qsTrId("main.import_from_url")
            onClicked: console.warn("Import recipe page is not implemented yet")
        }

        MenuItem {
            //% "Refresh"
            text: qsTrId("main.refresh")
            onClicked: refresh()
        }
    }

    SearchField {
        id: search
        width: parent.width - Theme.horizontalPageMargin * 2
        x: Theme.horizontalPageMargin
        //% "Search recipes"
        placeholderText: qsTrId("main.search_recipes")

        EnterKey.iconSource: "image://theme/icon-m-enter-accept"
        EnterKey.onClicked: focus = false

        onTextChanged: refreshTimer.restart()
    }

    StandardLabel {
        text: errorText
        color: Theme.errorColor
        visible: text.length > 0
    }

    StandardLabel {
        //% "No recipes"
        text: qsTrId("main.empty")
        color: Theme.secondaryColor
        visible: !errorText && recipes.length === 0
        horizontalAlignment: Text.AlignHCenter
    }

    Repeater {
        model: recipes

        RecipeListItem {
            width: page.width
            recipe: modelData

            onClicked: console.warn("Recipe detail page is not implemented yet")
            onEditRequested: console.warn("Recipe edit page is not implemented yet")
            onShareRequested: console.warn("Recipe sharing is not implemented yet")
            onDeleteRequested: {
                errorText = "";
                core.deleteRecipe(recipe.id);
            }
        }
    }

    Timer {
        id: refreshTimer
        interval: 300
        repeat: false
        onTriggered: refresh()
    }

    Component.onCompleted: {
        refresh();
        safeCall(function() {
            pageStack.pushAttached("CategoriesPage.qml");
        });
    }
}
