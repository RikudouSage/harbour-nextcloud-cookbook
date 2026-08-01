#ifndef CORE_H
#define CORE_H

#include <QObject>
#include <QJsonArray>
#include <QJsonObject>

#include "secrets.h"
#include "libcookbook.h"

class Core : public QObject
{
    Q_OBJECT
public:
    explicit Core(Secrets *secrets, QObject *parent = nullptr);

public:
    Q_INVOKABLE void validateCredentials(const QString &url, const QString &username, const QString &password);
    Q_INVOKABLE void reinitialize();
    Q_INVOKABLE void listRecipes();
    Q_INVOKABLE void searchRecipes(const QString &query);
    Q_INVOKABLE void listCategoryRecipes(const QString &category);
    Q_INVOKABLE void listKeywordRecipes(const QJsonArray &keywords);
    Q_INVOKABLE void listCategories();
    Q_INVOKABLE void listKeywords();
    Q_INVOKABLE void importRecipe(const QString &url);
    Q_INVOKABLE void deleteRecipe(const QString &id);

signals:
    void credentialsValidated(bool success);
    void credentialsValidated(bool success, const QString &url, const QString &username, const QString &password);
    void initialized(bool success);
    void recipesResolved(bool success, const QJsonArray &recipes);
    void categoriesResolved(bool success, const QJsonArray &categories);
    void keywordsResolved(bool success, const QJsonArray &keywords);
    void recipeImported(bool success, const QJsonObject &recipe);
    void recipeDeleted(bool success);

private:
    Secrets *secrets;
    bool valid;

    ClientHandle client = 0;
    ContextHandle ctx = 0;

private:
    void initialize();
    void cleanup();
    const QString getLastError() const;
    QJsonArray mapRecipes(const CookbookRecipeStubSlice &recipes) const;
    QJsonObject mapRecipeStub(const CookbookRecipeStub &recipe) const;
    QJsonObject mapRecipe(const CookbookRecipe &recipe) const;
    QJsonArray mapCategories(const CookbookCategorySlice &categories) const;
    QJsonArray mapKeywords(const CookbookKeywordSlice &keywords) const;
    QJsonArray mapStringSlice(const StringSlice &slice) const;
};

#endif // CORE_H
