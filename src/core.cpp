#include "core.h"
#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QImageReader>
#include <QMetaType>
#include <QStandardPaths>
#include <QStringList>
#include <QUrl>

#include <vector>
#include <QtConcurrent>

namespace {

constexpr qint64 RecipeImageCacheTTLSeconds = 60 * 60;

QString cString(const char *value)
{
    return value == nullptr ? QString() : QString::fromUtf8(value);
}

bool isContextCanceledError(const QString &error)
{
    return error.contains("context canceled", Qt::CaseInsensitive)
           || error.contains("context cancelled", Qt::CaseInsensitive);
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

QString Core::createContext()
{
    ContextHandle context = 0;
    if (CookbookNewContext(&context) != CookbookSuccess) {
        qWarning() << "Failed creating context:" << getLastError();
        return {};
    }

    return contextToString(context);
}

void Core::freeContext(const QString &context)
{
    if (context.isEmpty() || context == "0") {
        return;
    }

    const auto handle = contextFromString(context);
    if (handle == 0) {
        return;
    }

    CookbookCloseHandle(handle);
}

void Core::listRecipes()
{
    QtConcurrent::run([=] {
        CookbookRecipeStubSlice recipes = {};
        if (CookbookListRecipes(ctx, client, &recipes) != CookbookSuccess) {
            qWarning() << "Failed listing recipes: " << getLastError();
            emit recipesResolved("0", false, {});
            return;
        }

        const auto result = mapRecipes(recipes);
        CookbookFreeRecipeStubSlice(&recipes);
        emit recipesResolved("0", true, result);
    });
}

void Core::searchRecipes(const QString &query)
{
    searchRecipes(contextToString(ctx), query);
}

void Core::searchRecipes(const QString &context, const QString &query)
{
    QtConcurrent::run([=] {
        const auto searchContext = contextFromString(context);
        if (searchContext == 0) {
            qWarning() << "Cannot search recipes with invalid context";
            emit recipesResolved(context, false, {});
            return;
        }

        auto queryData = query.toUtf8();

        CookbookRecipeStubSlice recipes = {};
        if (CookbookSearchRecipes(searchContext, client, queryData.data(), &recipes) != CookbookSuccess) {
            const auto error = getLastError();
            if (isContextCanceledError(error)) {
                return;
            }

            qWarning() << "Failed searching recipes: " << error;
            emit recipesResolved(context, false, {});
            return;
        }

        const auto result = mapRecipes(recipes);
        CookbookFreeRecipeStubSlice(&recipes);
        emit recipesResolved(context, true, result);
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
            emit recipesResolved("0", false, {});
            return;
        }

        const auto result = mapRecipes(recipes);
        CookbookFreeRecipeStubSlice(&recipes);
        emit recipesResolved("0", true, result);
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
            emit recipesResolved("0", false, {});
            return;
        }

        const auto result = mapRecipes(recipes);
        CookbookFreeRecipeStubSlice(&recipes);
        emit recipesResolved("0", true, result);
    });
}

void Core::listKeywordRecipes(const QString &keyword)
{
    listKeywordRecipes(QJsonArray{keyword});
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

void Core::renameCategory(const QString &category, const QString &newName)
{
    QtConcurrent::run([=] {
        auto categoryData = category.toUtf8();
        auto newNameData = newName.toUtf8();
        CookbookCategory categoryInput = {};
        categoryInput.name = categoryData.data();

        CookbookCategory renamed = {};
        if (CookbookRenameCategory(ctx, client, &categoryInput, newNameData.data(), &renamed) != CookbookSuccess) {
            qWarning() << "Failed renaming category: " << getLastError();
            emit categoryRenamed(false, {});
            return;
        }

        const auto result = mapCategory(renamed);
        CookbookFreeCategory(&renamed);
        emit categoryRenamed(true, result);
    });
}

void Core::getRecipe(const QString &id)
{
    QtConcurrent::run([=] {
        auto idData = id.toUtf8();

        CookbookRecipe recipe = {};
        if (CookbookGetRecipe(ctx, client, idData.data(), &recipe) != CookbookSuccess) {
            qWarning() << "Failed getting recipe: " << getLastError();
            emit recipeResolved(false, {});
            return;
        }

        const auto result = mapRecipe(recipe);
        CookbookFreeRecipe(&recipe);
        emit recipeResolved(true, result);
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

void Core::resolveRecipeImage(const QString &id)
{
    QtConcurrent::run([=] {
        const auto cachedPath = cachedRecipeImagePath(id);
        if (!cachedPath.isEmpty()) {
            if (!isCachedRecipeImageValid(id, cachedPath)) {
                QFile::remove(cachedPath);
            } else {
                emit recipeImageResolved(true, id, QUrl::fromLocalFile(cachedPath).toString());
                return;
            }
        }

        auto idData = id.toUtf8();
        ByteSlice image = {};
        if (CookbookGetRecipeImage(ctx, client, idData.data(), nullptr, &image) != CookbookSuccess) {
            qWarning() << "Failed fetching recipe image:" << getLastError();
            emit recipeImageResolved(false, id, {});
            return;
        }

        QByteArray data(reinterpret_cast<const char *>(image.items), static_cast<int>(image.len));
        CookbookFreeBytes(&image);

        const auto extension = detectImageExtension(data);
        if (extension.isEmpty()) {
            qWarning() << "Could not detect image type for recipe" << id
                       << "bytes" << data.size()
                       << "prefix" << data.left(32).toHex();
            emit recipeImageResolved(false, id, {});
            return;
        }

        const auto path = recipeImageCachePath(id, extension);
        QDir().mkpath(QFileInfo(path).absolutePath());

        const auto tempPath = path + ".tmp";
        QFile file(tempPath);
        if (!file.open(QIODevice::WriteOnly) || file.write(data) != data.size()) {
            qWarning() << "Failed caching recipe image:" << tempPath << file.errorString();
            file.remove();
            emit recipeImageResolved(false, id, {});
            return;
        }
        file.close();

        QFile::remove(path);
        if (!QFile::rename(tempPath, path)) {
            qWarning() << "Failed moving cached recipe image into place:" << path;
            QFile::remove(tempPath);
            emit recipeImageResolved(false, id, {});
            return;
        }

        emit recipeImageResolved(true, id, QUrl::fromLocalFile(path).toString());
    });
}

void Core::invalidateRecipeImage(const QString &id)
{
    removeCachedRecipeImages(id);
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

ContextHandle Core::contextFromString(const QString &context) const
{
    bool ok = false;
    const auto handle = context.toULongLong(&ok);
    if (!ok) {
        return 0;
    }

    return static_cast<ContextHandle>(handle);
}

QString Core::contextToString(ContextHandle context) const
{
    return QString::number(static_cast<qulonglong>(context));
}

QJsonArray Core::mapRecipes(const CookbookRecipeStubSlice &recipes) const
{
    QJsonArray result;
    for (size_t i = 0; i < recipes.len; ++i) {
        result.append(mapRecipeStub(recipes.items[i]));
    }
    return result;
}

QString Core::cachedRecipeImagePath(const QString &id) const
{
    const QStringList extensions = {"png", "svg", "jpg", "webp", "gif"};
    const auto oldestValid = QDateTime::currentDateTimeUtc().addSecs(-RecipeImageCacheTTLSeconds);

    for (const auto &extension : extensions) {
        const auto path = recipeImageCachePath(id, extension);
        const QFileInfo fileInfo(path);
        if (!fileInfo.exists()) {
            continue;
        }

        if (fileInfo.lastModified().toUTC() >= oldestValid) {
            return path;
        }

        QFile::remove(path);
    }

    return {};
}

void Core::removeCachedRecipeImages(const QString &id) const
{
    const QStringList extensions = {"png", "svg", "jpg", "webp", "gif"};
    for (const auto &extension : extensions) {
        QFile::remove(recipeImageCachePath(id, extension));
        QFile::remove(recipeImageCachePath(id, extension) + ".tmp");
    }
}

bool Core::isCachedRecipeImageValid(const QString &id, const QString &path) const
{
    const QFileInfo fileInfo(path);
    if (!fileInfo.exists() || fileInfo.size() < 1) {
        qWarning() << "Recipe image cache file is empty or missing" << id << path << fileInfo.size();
        return false;
    }

    QImageReader reader(path);
    if (!reader.canRead()) {
        qWarning() << "Recipe image cache file is not readable" << id << path << reader.errorString();
        return false;
    }

    return true;
}

QString Core::recipeImageCachePath(const QString &id, const QString &extension) const
{
    auto cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (cacheDir.isEmpty()) {
        cacheDir = QDir::temp().filePath("harbour-nextcloud-cookbook");
    }
    return QDir(cacheDir).filePath(QString("recipes/%1.%2").arg(cacheSafeID(id), extension));
}

QString Core::detectImageExtension(const QByteArray &data) const
{
    const QByteArray pngSignature("\x89PNG\r\n\x1a\n", 8);
    if (data.startsWith(pngSignature)) {
        return "png";
    }

    const auto trimmed = data.trimmed();
    if (trimmed.startsWith("<svg") || (trimmed.startsWith("<?xml") && trimmed.contains("<svg"))) {
        return "svg";
    }

    if (data.startsWith("\xff\xd8\xff")) {
        return "jpg";
    }

    if (data.size() >= 12 && data.mid(0, 4) == "RIFF" && data.mid(8, 4) == "WEBP") {
        return "webp";
    }

    if (data.startsWith("GIF87a") || data.startsWith("GIF89a")) {
        return "gif";
    }

    return {};
}

QString Core::cacheSafeID(const QString &id) const
{
    QString safe;
    safe.reserve(id.size());

    for (const auto character : id) {
        if (character.isLetterOrNumber() || character == '-' || character == '_') {
            safe.append(character);
        } else {
            safe.append('_');
        }
    }

    return safe;
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

QJsonObject Core::mapCategory(const CookbookCategory &category) const
{
    QJsonObject result;
    result.insert("name", cString(category.name));
    result.insert("recipeCount", static_cast<int>(category.recipeCount));
    return result;
}

QJsonArray Core::mapCategories(const CookbookCategorySlice &categories) const
{
    QJsonArray result;
    for (size_t i = 0; i < categories.len; ++i) {
        result.append(mapCategory(categories.items[i]));
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
