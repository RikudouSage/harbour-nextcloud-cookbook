import QtQuick 2.0
import Sailfish.Silica 1.0

import "../components"

StandardPage {
    property string categoryName
    property string errorText
    property var recipes: []

    id: page
    title: displayCategoryName(categoryName)

    function displayCategoryName(name) {
        if (name === "*") {
            //% "Uncategorized"
            return qsTrId("categories.uncategorized");
        }

        return name || "";
    }

    function refresh(showLoading) {
        errorText = "";

        if (showLoading !== false) {
            loading = true;
            //% "Loading recipes..."
            loadText = qsTrId("main.loading_recipes");
        }

        core.listCategoryRecipes(categoryName);
    }

    function pushRenameCategory() {
        const dialog = pageStack.push("RenameCategoryPage.qml", {
            categoryName: displayCategoryName(page.categoryName)
        });

        dialog.accepted.connect(function() {
            errorText = "";
            loading = true;
            //% "Renaming category..."
            loadText = qsTrId("category_detail.renaming");
            core.renameCategory(page.categoryName, dialog.categoryName.trim());
        });
    }

    Connections {
        target: core

        onRecipesResolved: {
            if (page.status !== PageStatus.Active && page.status !== PageStatus.Activating) {
                return;
            }

            core.freeContext(context);
            loading = false;

            if (!success) {
                //% "Could not load recipes. Pull down to refresh."
                errorText = qsTrId("main.recipes_error");
                return;
            }

            page.recipes = recipes;
        }

        onRecipeDeleted: {
            if (!success) {
                //% "Could not delete recipe."
                errorText = qsTrId("main.delete_error");
                return;
            }

            refresh(false);
        }

        onCategoryRenamed: {
            loading = false;

            if (!success) {
                //% "Could not rename category."
                errorText = qsTrId("category_detail.rename_error");
                return;
            }

            categoryName = category.name || "";
            refresh();
        }
    }

    PullDownMenu {
        MenuItem {
            //% "Rename category"
            text: qsTrId("category_detail.rename")
            onClicked: pushRenameCategory()
        }

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

    Component.onCompleted: refresh()
}
