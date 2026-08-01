import QtQuick 2.0
import Nemo.KeepAlive 1.2
import Sailfish.Silica 1.0

import "../components"

StandardPage {
    property string recipeId
    property var recipe: ({})
    property string errorText
    property string imageSource
    property bool imageRequested: false

    id: page
    title: recipe.name || ""

    DisplayBlanking {
        id: displayBlanking
        preventBlanking: secrets.keepScreenOn
                          && (page.status === PageStatus.Active
                              || page.status === PageStatus.Activating)
    }

    function refresh() {
        if (!recipeId) {
            return;
        }

        errorText = "";
        loading = true;
        //% "Loading recipe..."
        loadText = qsTrId("recipe_detail.loading");
        core.getRecipe(recipeId);
    }

    function loadImage() {
        if (imageRequested || !recipe.id) {
            return;
        }

        imageRequested = true;
        core.resolveRecipeImage(recipe.id);
    }

    function formatDate(value) {
        if (!value) {
            return "";
        }

        return Format.formatDate(new Date(value), Format.DateMedium);
    }

    function formatDuration(value) {
        if (!value) {
            return "";
        }

        const match = /^P(?:T)?(?:(\d+)H)?(?:(\d+)M)?$/.exec(value);
        if (!match) {
            return value;
        }

        const parts = [];
        if (match[1]) {
            parts.push(match[1] + "h");
        }
        if (match[2]) {
            parts.push(match[2] + "m");
        }
        return parts.join(" ");
    }

    function hasItems(items) {
        return !!items && items.length > 0;
    }

    function hasText(value) {
        return !!value && value.length > 0;
    }

    function keywords() {
        return (recipe.keywords || []).filter(function(keyword) {
            return hasText(keyword);
        });
    }

    function displayCategoryName(name) {
        if (name === "*") {
            //% "Uncategorized"
            return qsTrId("categories.uncategorized");
        }

        return name || "";
    }

    function hasNutrition() {
        const nutrition = recipe.nutrition || {};
        return hasText(nutrition.calories)
                || hasText(nutrition.carbohydrateContent)
                || hasText(nutrition.cholesterolContent)
                || hasText(nutrition.fatContent)
                || hasText(nutrition.fiberContent)
                || hasText(nutrition.proteinContent)
                || hasText(nutrition.saturatedFatContent)
                || hasText(nutrition.servingSize)
                || hasText(nutrition.sodiumContent)
                || hasText(nutrition.sugarContent)
                || hasText(nutrition.transFatContent)
                || hasText(nutrition.unsaturatedFatContent);
    }

    function nutritionItems() {
        const nutrition = recipe.nutrition || {};
        return [
            //% "Calories"
            { label: qsTrId("recipe_detail.nutrition.calories"), value: nutrition.calories },
            //% "Carbohydrate"
            { label: qsTrId("recipe_detail.nutrition.carbohydrate"), value: nutrition.carbohydrateContent },
            //% "Cholesterol"
            { label: qsTrId("recipe_detail.nutrition.cholesterol"), value: nutrition.cholesterolContent },
            //% "Fat"
            { label: qsTrId("recipe_detail.nutrition.fat"), value: nutrition.fatContent },
            //% "Fiber"
            { label: qsTrId("recipe_detail.nutrition.fiber"), value: nutrition.fiberContent },
            //% "Protein"
            { label: qsTrId("recipe_detail.nutrition.protein"), value: nutrition.proteinContent },
            //% "Saturated fat"
            { label: qsTrId("recipe_detail.nutrition.saturated_fat"), value: nutrition.saturatedFatContent },
            //% "Serving size"
            { label: qsTrId("recipe_detail.nutrition.serving_size"), value: nutrition.servingSize },
            //% "Sodium"
            { label: qsTrId("recipe_detail.nutrition.sodium"), value: nutrition.sodiumContent },
            //% "Sugar"
            { label: qsTrId("recipe_detail.nutrition.sugar"), value: nutrition.sugarContent },
            //% "Trans fat"
            { label: qsTrId("recipe_detail.nutrition.trans_fat"), value: nutrition.transFatContent },
            //% "Unsaturated fat"
            { label: qsTrId("recipe_detail.nutrition.unsaturated_fat"), value: nutrition.unsaturatedFatContent }
        ].filter(function(item) {
            return hasText(item.value);
        });
    }

    function timeItems() {
        return [
            //% "Preparation"
            { label: qsTrId("recipe_detail.preparation_time"), value: formatDuration(recipe.preparationTime) },
            //% "Cooking"
            { label: qsTrId("recipe_detail.cooking_time"), value: formatDuration(recipe.cookTime) },
            //% "Total time"
            { label: qsTrId("recipe_detail.total_time"), value: formatDuration(recipe.totalTime) }
        ].filter(function(item) {
            return hasText(item.value);
        });
    }

    function metadataItems() {
        return [
            //% "Created"
            { label: qsTrId("recipe_detail.created"), value: formatDate(recipe.createdDate) },
            //% "Modified"
            { label: qsTrId("recipe_detail.modified"), value: formatDate(recipe.modifiedDate) },
            //% "Category"
            { label: qsTrId("recipe_detail.category"), value: displayCategoryName(recipe.category) }
        ].filter(function(item) {
            return hasText(item.value);
        });
    }

    function sourceText() {
        if (!hasText(recipe.url)) {
            return "";
        }

        //% "Source: %1"
        return qsTrId("recipe_detail.source").arg(
                    "<a href=\"" + recipe.url + "\" style=\"color: "
                    + Theme.highlightColor
                    + "; text-decoration: underline;\">"
                    + recipe.url
                    + "</a>");
    }

    Connections {
        target: core

        onRecipeResolved: {
            if (page.status !== PageStatus.Active && page.status !== PageStatus.Activating) {
                return;
            }

            loading = false;

            if (!success) {
                //% "Could not load recipe. Pull down to refresh."
                errorText = qsTrId("recipe_detail.error");
                return;
            }

            page.recipe = recipe;
            page.recipeId = recipe.id || page.recipeId;
            page.imageSource = "";
            page.imageRequested = false;
            loadImage();
        }

        onRecipeImageResolved: {
            if (!recipe.id || id !== recipe.id) {
                return;
            }

            if (success) {
                imageSource = path;
            }
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

    Rectangle {
        x: Theme.horizontalPageMargin
        width: parent.width - Theme.horizontalPageMargin * 2
        height: visible ? Math.round(width * 0.58) : 0
        color: Theme.rgba(Theme.highlightBackgroundColor, 0.1)
        visible: recipe.id && (imageSource || recipe.imageUrl || recipe.image || recipe.imagePlaceholderUrl)

        Image {
            id: recipeImage
            anchors.fill: parent
            fillMode: Image.PreserveAspectCrop
            source: imageSource
            visible: status === Image.Ready
        }

        Image {
            anchors.centerIn: parent
            source: "image://theme/icon-m-file-image?" + Theme.secondaryColor
            visible: !recipeImage.visible
        }
    }

    Label {
        x: Theme.horizontalPageMargin
        width: parent.width - Theme.horizontalPageMargin * 2
        color: Theme.primaryColor
        font.pixelSize: Theme.fontSizeExtraLarge
        wrapMode: Label.WordWrap
        text: recipe.name || ""
        visible: text.length > 0
    }

    Flow {
        x: Theme.horizontalPageMargin
        width: parent.width - Theme.horizontalPageMargin * 2
        spacing: Theme.paddingSmall
        visible: hasItems(keywords())

        Repeater {
            model: keywords()

            Label {
                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeExtraSmall
                text: "#" + modelData
            }
        }
    }

    Column {
        x: Theme.horizontalPageMargin
        width: parent.width - Theme.horizontalPageMargin * 2
        spacing: Theme.paddingSmall
        visible: metadataItems().length > 0

        Repeater {
            model: metadataItems()

            Label {
                width: parent.width
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeExtraSmall
                wrapMode: Label.WordWrap
                text: modelData.label + ": " + modelData.value
            }
        }
    }

    TextSwitch {
        //% "Keep screen on"
        text: qsTrId("recipe_detail.keep_screen_on")
        checked: secrets.keepScreenOn
        onCheckedChanged: secrets.keepScreenOn = checked
    }

    StandardLabel {
        text: recipe.description || ""
        color: Theme.secondaryColor
        visible: text.length > 0
    }

    Label {
        x: Theme.horizontalPageMargin
        width: parent.width - Theme.horizontalPageMargin * 2
        text: sourceText()
        color: Theme.secondaryColor
        textFormat: Text.RichText
        linkColor: Theme.highlightColor
        wrapMode: Label.WordWrap
        visible: hasText(recipe.url)
        onLinkActivated: Qt.openUrlExternally(link)
    }

    StandardLabel {
        //% "Servings: %1"
        text: qsTrId("recipe_detail.servings").arg(recipe.servings || 0)
        color: Theme.secondaryColor
        visible: recipe.servings > 0
    }

    SectionHeader {
        //% "Durations"
        text: qsTrId("recipe_detail.durations")
        visible: timeItems().length > 0
    }

    Row {
        property var items: timeItems()
        property real badgeWidth: (page.width - Theme.horizontalPageMargin * 2 - spacing * 2) / 3

        spacing: Theme.paddingLarge
        width: badgeWidth * items.length + spacing * Math.max(0, items.length - 1)
        x: (parent.width - width) / 2
        visible: items.length > 0

        Repeater {
            model: parent.items

            Rectangle {
                width: parent.badgeWidth
                height: durationColumn.height + Theme.paddingMedium * 2
                radius: Theme.paddingSmall
                color: Theme.rgba(Theme.highlightBackgroundColor, 0.08)
                border.color: Theme.rgba(Theme.highlightColor, 0.25)
                border.width: 1

                Column {
                    id: durationColumn
                    anchors {
                        left: parent.left
                        right: parent.right
                        verticalCenter: parent.verticalCenter
                        margins: Theme.paddingMedium
                    }
                    spacing: Theme.paddingSmall

                    Label {
                        width: parent.width
                        color: Theme.secondaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Label.WordWrap
                        text: modelData.label
                    }

                    Label {
                        width: parent.width
                        color: Theme.primaryColor
                        horizontalAlignment: Text.AlignHCenter
                        text: modelData.value
                    }
                }
            }
        }
    }

    SectionHeader {
        //% "Ingredients"
        text: qsTrId("recipe_detail.ingredients")
        visible: hasItems(recipe.ingredients)
    }

    Column {
        x: Theme.horizontalPageMargin
        width: parent.width - Theme.horizontalPageMargin * 2
        spacing: Theme.paddingSmall
        visible: hasItems(recipe.ingredients)

        Repeater {
            model: recipe.ingredients || []

            Label {
                width: parent.width
                color: Theme.primaryColor
                wrapMode: Label.WordWrap
                text: modelData
            }
        }
    }

    SectionHeader {
        //% "Instructions"
        text: qsTrId("recipe_detail.instructions")
        visible: hasItems(recipe.instructions)
    }

    Column {
        width: parent.width
        spacing: Theme.paddingSmall
        visible: hasItems(recipe.instructions)

        Repeater {
            model: recipe.instructions || []

            ExpandingSectionGroup {
                id: instructionGroup

                width: parent.width
                currentIndex: 0

                ExpandingSection {
                    //% "Step %1"
                    property string stepTitle: qsTrId("recipe_detail.step").arg(index + 1)

                    title: (instructionGroup.currentIndex < 0 ? "✓ " : "") + stepTitle

                    content.sourceComponent: Column {
                        Label {
                            x: Theme.horizontalPageMargin
                            width: parent.width - Theme.horizontalPageMargin * 2
                            color: Theme.primaryColor
                            wrapMode: Label.WordWrap
                            text: modelData
                        }
                    }
                }
            }
        }
    }

    SectionHeader {
        //% "Tools"
        text: qsTrId("recipe_detail.tools")
        visible: hasItems(recipe.tools)
    }

    Column {
        x: Theme.horizontalPageMargin
        width: parent.width - Theme.horizontalPageMargin * 2
        spacing: Theme.paddingSmall
        visible: hasItems(recipe.tools)

        Repeater {
            model: recipe.tools || []

            Label {
                width: parent.width
                color: Theme.primaryColor
                wrapMode: Label.WordWrap
                text: modelData
            }
        }
    }

    SectionHeader {
        //% "Nutrition Information"
        text: qsTrId("recipe_detail.nutrition")
        visible: hasNutrition()
    }

    Column {
        x: Theme.horizontalPageMargin
        width: parent.width - Theme.horizontalPageMargin * 2
        spacing: Theme.paddingMedium
        visible: hasNutrition()

        Repeater {
            model: nutritionItems()

            Column {
                width: parent.width
                spacing: Theme.paddingSmall

                Label {
                    width: parent.width
                    color: Theme.primaryColor
                    font.bold: true
                    wrapMode: Label.WordWrap
                    text: modelData.label
                }

                Label {
                    width: parent.width
                    color: Theme.secondaryColor
                    wrapMode: Label.WordWrap
                    text: modelData.value
                }
            }
        }
    }

    StandardLabel {
        //% "No recipe details"
        text: qsTrId("recipe_detail.empty")
        color: Theme.secondaryColor
        horizontalAlignment: Text.AlignHCenter
        visible: recipe.id
                 && !hasText(recipe.name)
                 && !hasText(recipe.description)
                 && !hasText(recipe.url)
                 && !hasItems(keywords())
                 && metadataItems().length === 0
                 && timeItems().length === 0
                 && recipe.servings <= 0
                 && !hasItems(recipe.ingredients)
                 && !hasItems(recipe.instructions)
                 && !hasItems(recipe.tools)
                 && !hasNutrition()
    }

    Component.onCompleted: refresh()

    Component.onDestruction: {
        displayBlanking.preventBlanking = false;
    }
}
