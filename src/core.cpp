#include "core.h"
#include <QDebug>
#include <QMetaType>

#include <vector>
#include <QtConcurrent>

namespace {

QString cString(const char *value)
{
    return value == nullptr ? QString() : QString::fromUtf8(value);
}

}

Core::Core(Secrets *secrets, QObject *parent)
    : QObject(parent), secrets(secrets)
{
    qRegisterMetaType<QJsonArray>("QJsonArray");
    qRegisterMetaType<QJsonObject>("QJsonObject");
    initialize();
}

void Core::initialize()
{
    valid = true;
    cleanup();

    if (CookbookNewContext(&ctx) != CookbookSuccess) {
        valid = false;
        qWarning() << "Failed creating context: " << getLastError();
    }

    auto url = secrets->nextcloudUrl().toUtf8();
    auto username = secrets->username().toUtf8();
    auto password = secrets->password().toUtf8();

    if (CookbookNewClient(&client, NewClientOptions {
        .url = url.data(),
        .username = username.data(),
        .password = password.data(),
    }) != CookbookSuccess) {
        valid = false;
        qWarning() << "Failed creating client: " << getLastError();
    }
}

void Core::validateCredentials(const QString &url, const QString &username, const QString &password)
{
    QtConcurrent::run([=] {
        auto urlData = url.toUtf8();
        auto usernameData = username.toUtf8();
        auto passwordData = password.toUtf8();

        ClientHandle tempClient;
        if (CookbookNewClient(&tempClient, NewClientOptions {
            .url = urlData.data(),
            .username = usernameData.data(),
            .password = passwordData.data(),
        }) != CookbookSuccess) {
            qWarning() << "Failed creating temporary client: " << getLastError();
            emit credentialsValidated(false);
            return;
        }

        auto close = [=] {
            if (CookbookCloseHandle(tempClient) != CookbookSuccess) {
                qWarning() << "Failed closing temporary client: " << getLastError();
            }
        };

        bool success = false;
        if (CookbookValidateCredentials(ctx, tempClient, &success) != CookbookSuccess) {
            qWarning() << "Failed validating credentials: " << getLastError();
            close();
            emit credentialsValidated(false);
            return;
        }

        close();
        emit credentialsValidated(success, url, username, password);
    });
}

void Core::listRecipes()
{
    QtConcurrent::run([=] {
        CookbookRecipeStubSlice recipes = {};
        if (CookbookListRecipes(ctx, client, &recipes) != CookbookSuccess) {
            qWarning() << "Failed listing recipes: " << getLastError();
            emit recipesResolved(false, {});
            return;
        }

        const auto result = mapRecipes(recipes);
        CookbookFreeRecipeStubSlice(&recipes);
        emit recipesResolved(true, result);
    });
}

void Core::searchRecipes(const QString &query)
{
    QtConcurrent::run([=] {
        auto queryData = query.toUtf8();

        CookbookRecipeStubSlice recipes = {};
        if (CookbookSearchRecipes(ctx, client, queryData.data(), &recipes) != CookbookSuccess) {
            qWarning() << "Failed searching recipes: " << getLastError();
            emit recipesResolved(false, {});
            return;
        }

        const auto result = mapRecipes(recipes);
        CookbookFreeRecipeStubSlice(&recipes);
        emit recipesResolved(true, result);
    });
}

void Core::listCategoryRecipes(const QString &category)
{
    QtConcurrent::run([=] {
        auto categoryData = category.toUtf8();
        CookbookCategory categoryInput = {};
        categoryInput.name = categoryData.data();

        CookbookRecipeStubSlice recipes = {};
        if (CookbookGetCategoryRecipes(ctx, client, &categoryInput, &recipes) != CookbookSuccess) {
            qWarning() << "Failed listing category recipes: " << getLastError();
            emit recipesResolved(false, {});
            return;
        }

        const auto result = mapRecipes(recipes);
        CookbookFreeRecipeStubSlice(&recipes);
        emit recipesResolved(true, result);
    });
}

void Core::listKeywordRecipes(const QJsonArray &keywords)
{
    QtConcurrent::run([=] {
        std::vector<QByteArray> keywordData;
        std::vector<char *> keywordPointers;
        keywordData.reserve(static_cast<size_t>(keywords.size()));
        keywordPointers.reserve(static_cast<size_t>(keywords.size()));

        for (const auto &keyword : keywords) {
            keywordData.push_back(keyword.toString().toUtf8());
        }
        for (auto &keyword : keywordData) {
            keywordPointers.push_back(keyword.data());
        }

        StringSlice keywordSlice = {};
        keywordSlice.items = keywordPointers.data();
        keywordSlice.len = keywordPointers.size();

        CookbookRecipeStubSlice recipes = {};
        if (CookbookGetKeywordRecipes(ctx, client, keywordSlice, &recipes) != CookbookSuccess) {
            qWarning() << "Failed listing keyword recipes: " << getLastError();
            emit recipesResolved(false, {});
            return;
        }

        const auto result = mapRecipes(recipes);
        CookbookFreeRecipeStubSlice(&recipes);
        emit recipesResolved(true, result);
    });
}

void Core::listCategories()
{
    QtConcurrent::run([=] {
        CookbookCategorySlice categories = {};
        if (CookbookListCategories(ctx, client, &categories) != CookbookSuccess) {
            qWarning() << "Failed listing categories: " << getLastError();
            emit categoriesResolved(false, {});
            return;
        }

        const auto result = mapCategories(categories);
        CookbookFreeCategorySlice(&categories);
        emit categoriesResolved(true, result);
    });
}

void Core::listKeywords()
{
    QtConcurrent::run([=] {
        CookbookKeywordSlice keywords = {};
        if (CookbookListKeywords(ctx, client, &keywords) != CookbookSuccess) {
            qWarning() << "Failed listing keywords: " << getLastError();
            emit keywordsResolved(false, {});
            return;
        }

        const auto result = mapKeywords(keywords);
        CookbookFreeKeywordSlice(&keywords);
        emit keywordsResolved(true, result);
    });
}

void Core::importRecipe(const QString &url)
{
    QtConcurrent::run([=] {
        auto urlData = url.toUtf8();

        CookbookRecipe recipe = {};
        if (CookbookImportRecipe(ctx, client, urlData.data(), &recipe) != CookbookSuccess) {
            qWarning() << "Failed importing recipe: " << getLastError();
            emit recipeImported(false, {});
            return;
        }

        const auto result = mapRecipe(recipe);
        CookbookFreeRecipe(&recipe);
        emit recipeImported(true, result);
    });
}

void Core::deleteRecipe(const QString &id)
{
    QtConcurrent::run([=] {
        auto idData = id.toUtf8();
        if (CookbookDeleteRecipe(ctx, client, idData.data()) != CookbookSuccess) {
            qWarning() << "Failed deleting recipe: " << getLastError();
            emit recipeDeleted(false);
            return;
        }

        emit recipeDeleted(true);
    });
}

void Core::reinitialize()
{
    QtConcurrent::run([=] {
        initialize();
        emit initialized(valid);
    });
}

void Core::cleanup()
{
    if (ctx != 0 && CookbookCloseHandle(ctx) != CookbookSuccess) {
        qWarning() << "Failed closing context: " << getLastError();
    }
    if (client != 0 && CookbookCloseHandle(client) != CookbookSuccess) {
        qWarning() << "Failed closing client: " << getLastError();
    }

    ctx = 0;
    client = 0;
}

const QString Core::getLastError() const
{
    std::size_t len = CookbookGetLastError(nullptr, 0);

    if (len < 1) {
        return QString();
    }

    QByteArray buf(static_cast<int>(len), Qt::Uninitialized);
    CookbookGetLastError(buf.data(), static_cast<std::size_t>(buf.size()));

    return QString::fromUtf8(buf.constData());
}

QJsonArray Core::mapRecipes(const CookbookRecipeStubSlice &recipes) const
{
    QJsonArray result;
    for (size_t i = 0; i < recipes.len; ++i) {
        result.append(mapRecipeStub(recipes.items[i]));
    }
    return result;
}

QJsonObject Core::mapRecipeStub(const CookbookRecipeStub &recipe) const
{
    QJsonObject outRecipe;
    outRecipe.insert("id", cString(recipe.id));
    outRecipe.insert("name", cString(recipe.name));
    outRecipe.insert("keywords", mapStringSlice(recipe.keywords));
    outRecipe.insert("createdDate", static_cast<double>(recipe.createdDate));
    outRecipe.insert("modifiedDate", static_cast<double>(recipe.modifiedDate));
    outRecipe.insert("imageUrl", cString(recipe.imageUrl));
    outRecipe.insert("imagePlaceholderUrl", cString(recipe.imagePlaceholderUrl));
    return outRecipe;
}

QJsonObject Core::mapRecipe(const CookbookRecipe &recipe) const
{
    QJsonObject outRecipe;
    outRecipe.insert("id", cString(recipe.id));
    outRecipe.insert("schemaType", cString(recipe.schemaType));
    outRecipe.insert("name", cString(recipe.name));
    outRecipe.insert("keywords", mapStringSlice(recipe.keywords));
    outRecipe.insert("createdDate", static_cast<double>(recipe.createdDate));
    outRecipe.insert("modifiedDate", static_cast<double>(recipe.modifiedDate));
    outRecipe.insert("imageUrl", cString(recipe.imageUrl));
    outRecipe.insert("imagePlaceholderUrl", cString(recipe.imagePlaceholderUrl));
    outRecipe.insert("preparationTime", cString(recipe.preparationTime));
    outRecipe.insert("cookTime", cString(recipe.cookTime));
    outRecipe.insert("totalTime", cString(recipe.totalTime));
    outRecipe.insert("description", cString(recipe.description));
    outRecipe.insert("url", cString(recipe.url));
    outRecipe.insert("image", cString(recipe.image));
    outRecipe.insert("servings", static_cast<int>(recipe.servings));
    outRecipe.insert("category", cString(recipe.category));
    outRecipe.insert("tools", mapStringSlice(recipe.tools));
    outRecipe.insert("ingredients", mapStringSlice(recipe.ingredients));
    outRecipe.insert("instructions", mapStringSlice(recipe.instructions));

    QJsonObject nutrition;
    nutrition.insert("type", cString(recipe.nutrition.type));
    nutrition.insert("calories", cString(recipe.nutrition.calories));
    nutrition.insert("carbohydrateContent", cString(recipe.nutrition.carbohydrateContent));
    nutrition.insert("cholesterolContent", cString(recipe.nutrition.cholesterolContent));
    nutrition.insert("fatContent", cString(recipe.nutrition.fatContent));
    nutrition.insert("fiberContent", cString(recipe.nutrition.fiberContent));
    nutrition.insert("proteinContent", cString(recipe.nutrition.proteinContent));
    nutrition.insert("saturatedFatContent", cString(recipe.nutrition.saturatedFatContent));
    nutrition.insert("servingSize", cString(recipe.nutrition.servingSize));
    nutrition.insert("sodiumContent", cString(recipe.nutrition.sodiumContent));
    nutrition.insert("sugarContent", cString(recipe.nutrition.sugarContent));
    nutrition.insert("transFatContent", cString(recipe.nutrition.transFatContent));
    nutrition.insert("unsaturatedFatContent", cString(recipe.nutrition.unsaturatedFatContent));
    outRecipe.insert("nutrition", nutrition);

    return outRecipe;
}

QJsonArray Core::mapCategories(const CookbookCategorySlice &categories) const
{
    QJsonArray result;
    for (size_t i = 0; i < categories.len; ++i) {
        const auto category = categories.items[i];
        QJsonObject outCategory;
        outCategory.insert("name", cString(category.name));
        outCategory.insert("recipeCount", static_cast<int>(category.recipeCount));
        result.append(outCategory);
    }
    return result;
}

QJsonArray Core::mapKeywords(const CookbookKeywordSlice &keywords) const
{
    QJsonArray result;
    for (size_t i = 0; i < keywords.len; ++i) {
        const auto keyword = keywords.items[i];
        QJsonObject outKeyword;
        outKeyword.insert("name", cString(keyword.name));
        outKeyword.insert("recipeCount", static_cast<int>(keyword.recipeCount));
        result.append(outKeyword);
    }
    return result;
}

QJsonArray Core::mapStringSlice(const StringSlice &slice) const
{
    QJsonArray result;
    for (size_t i = 0; i < slice.len; ++i) {
        result.append(cString(slice.items[i]));
    }
    return result;
}
