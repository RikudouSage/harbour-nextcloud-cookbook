import QtQuick 2.0
import Sailfish.Silica 1.0

import "../components"

StandardPage {
    //% "Recipes"
    property string scopeTitle: qsTrId("main.title")
    property var recipes: []
    property string errorText
    property bool searchLoading: false
    property string searchContext

    id: page
    title: scopeTitle

    function refresh(showLoading) {
        errorText = "";
        if (showLoading !== false) {
            loading = true;
            //% "Loading recipes..."
            loadText = qsTrId("main.loading_recipes");
        }

        core.listRecipes();
    }

    function searchRecipes() {
        errorText = "";
        if (search.text.length > 0) {
            resetSearchContext();
            searchContext = core.createContext();
            if (!searchContext) {
                //% "Could not search recipes."
                errorText = qsTrId("main.search_error");
                return;
            }

            searchLoading = true;
            core.searchRecipes(searchContext, search.text);
        } else {
            resetSearchContext();
            refresh();
        }
    }

    function resetSearchContext() {
        if (!searchContext) {
            return;
        }

        core.freeContext(searchContext);
        searchContext = "";
    }

    function logout() {
        secrets.password = "";
        core.reinitialize();
        safeCall(function() {
            pageStack.replace("CheckPage.qml");
        });
    }

    function pushImportRecipe() {
        const dialog = pageStack.push("ImportRecipePage.qml");

        dialog.accepted.connect(function() {
            errorText = "";
            loading = true;
            //% "Importing recipe..."
            loadText = qsTrId("import_recipe.importing");
            core.importRecipe(dialog.url.trim());
        });
    }

    Connections {
        target: core

        onRecipesResolved: {
            if (page.status !== PageStatus.Active && page.status !== PageStatus.Activating) {
                return;
            }

            if (searchContext && context !== searchContext) {
                return;
            }
            core.freeContext(context);

            loading = false;
            searchLoading = false;
            searchContext = "";

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
                searchLoading = false;
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

            refresh(false);
        }
    }

    PullDownMenu {
        MenuItem {
            //% "Logout"
            text: qsTrId("main.logout")
            onClicked: logout()
        }

//        MenuItem {
//            //% "Create recipe"
//            text: qsTrId("main.create_recipe")
//            onClicked: console.warn("Create recipe page is not implemented yet")
//        }

        MenuItem {
            //% "Import from URL"
            text: qsTrId("main.import_from_url")
            onClicked: pushImportRecipe()
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

    BusyLabel {
        //% "Loading recipes..."
        text: qsTrId("main.loading_recipes")
        running: searchLoading
        height: running ? implicitHeight : 0
        visible: running
    }

    StandardLabel {
        //% "No recipes"
        text: qsTrId("main.empty")
        color: Theme.secondaryColor
        visible: !searchLoading && !errorText && recipes.length === 0
        horizontalAlignment: Text.AlignHCenter
    }

    Repeater {
        model: searchLoading ? [] : recipes

        RecipeListItem {
            width: page.width
            recipe: modelData

            onClicked: pageStack.push("RecipeDetailPage.qml", {
                recipeId: recipe.id || "",
                recipe: recipe
            })
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
        onTriggered: searchRecipes()
    }

    Component.onCompleted: {
        refresh();
        safeCall(function() {
            pageStack.pushAttached("CategoriesPage.qml");
        });
    }
}
