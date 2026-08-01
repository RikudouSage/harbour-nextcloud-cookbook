#include "secrets.h"

#include <Sailfish/Secrets/collectionnamesrequest.h>
#include <Sailfish/Secrets/createcollectionrequest.h>
#include <Sailfish/Secrets/result.h>
#include <Sailfish/Secrets/secret.h>
#include <Sailfish/Secrets/storesecretrequest.h>
#include <Sailfish/Secrets/storedsecretrequest.h>
#include <Sailfish/Secrets/deletesecretrequest.h>
#include <Sailfish/Secrets/deletecollectionrequest.h>

#include <QDebug>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QSettings>

using Sailfish::Secrets::CollectionNamesRequest;
using Sailfish::Secrets::SecretManager;
using Sailfish::Secrets::Request;
using Sailfish::Secrets::Result;
using Sailfish::Secrets::CreateCollectionRequest;
using Sailfish::Secrets::Secret;
using Sailfish::Secrets::StoreSecretRequest;
using Sailfish::Secrets::StoredSecretRequest;
using Sailfish::Secrets::DeleteSecretRequest;
using Sailfish::Secrets::DeleteCollectionRequest;

const QString Secrets::collectionName(QStringLiteral("cookbook"));

constexpr auto secretNameUsername = "username";
constexpr auto secretNamePassword = "password";

#ifdef QT_DEBUG
static QSettings &insecureEmulatorSecrets()
{
    static QSettings settings(QStringLiteral("cz.chrastecky"), QStringLiteral("cookbook-insecure-emulator-secrets"));
    return settings;
}
#endif

Secrets::Secrets(QObject *parent) : QObject(parent)
{
#ifdef QT_DEBUG
    qWarning() << "Using insecure debug secrets storage. Do not enable this in release builds.";
#else
    CollectionNamesRequest cnr;
    cnr.setManager(secretManager);
    cnr.setStoragePluginName(SecretManager::DefaultEncryptedStoragePluginName);
    cnr.startRequest();
    cnr.waitForFinished();

    hasBitsailorCollection = isResultValid(cnr) && cnr.collectionNames().contains(collectionName);
#endif
}

QString Secrets::username()
{
    return getData(secretNameUsername);
}

QString Secrets::password()
{
    return getData(secretNamePassword);
}

void Secrets::setUsername(const QString &username)
{
    const auto current = this->username();
    if (username == current) {
        return;
    }

    storeData(secretNameUsername, username);
    emit usernameChanged();
}

void Secrets::setPassword(const QString &password)
{
    const auto current = this->password();
    if (password == current) {
        return;
    }

    storeData(secretNamePassword, password);
    emit passwordChanged();
}

bool Secrets::clearAllSecrets()
{
#ifdef QT_DEBUG
    auto &settings = insecureEmulatorSecrets();
    settings.clear();
    settings.sync();
    return settings.status() == QSettings::NoError;
#else
    DeleteCollectionRequest dcr;
    dcr.setCollectionName(collectionName);
    dcr.setStoragePluginName(SecretManager::DefaultEncryptedStoragePluginName);
    dcr.setUserInteractionMode(SecretManager::SystemInteraction);
    dcr.setManager(secretManager);
    dcr.startRequest();
    dcr.waitForFinished();

    auto success = isResultValid(dcr);

    hasBitsailorCollection = !success;
    return success;
#endif
}

bool Secrets::isResultValid(const Request &request)
{
    auto result = request.result();
    auto isSuccess = result.errorCode() == Result::NoError;
    if (!isSuccess) {
        qWarning() << result.errorMessage();
    }

    return isSuccess;
}

bool Secrets::isSecretValid(const Secret &secret)
{
    return !secret.name().isNull() && !secret.name().isEmpty();
}

bool Secrets::storeData(const QString &name, const QString &data)
{
#ifdef QT_DEBUG
    auto &settings = insecureEmulatorSecrets();
    settings.setValue(name, data);
    settings.sync();
    return settings.status() == QSettings::NoError;
#else
    if (!hasBitsailorCollection) {
        createCollection();
        // todo handle case where collection isn't created
    }

    auto existingSecret = getSecret(name);
    if (isSecretValid(existingSecret)) {
        deleteSecret(name);
    }

    Secret secret(toIdentifier(name));
    secret.setData(data.toUtf8());

    StoreSecretRequest ssr;
    ssr.setManager(secretManager);
    ssr.setSecretStorageType(StoreSecretRequest::CollectionSecret);
    ssr.setUserInteractionMode(SecretManager::SystemInteraction);
    ssr.setSecret(secret);
    ssr.startRequest();
    ssr.waitForFinished();

    return isResultValid(ssr);
#endif
}

Secret Secrets::getSecret(const QString &name)
{
    if (!hasBitsailorCollection) {
        return Secret();
    }

    StoredSecretRequest ssr;
    ssr.setManager(secretManager);
    ssr.setUserInteractionMode(SecretManager::SystemInteraction);
    ssr.setIdentifier(toIdentifier(name));
    ssr.startRequest();
    ssr.waitForFinished();

    auto success = isResultValid(ssr);
    if (!success) {
        return Secret();
    }

    return ssr.secret();
}

bool Secrets::deleteSecret(const QString &name)
{
#ifdef QT_DEBUG
    auto &settings = insecureEmulatorSecrets();
    settings.remove(name);
    settings.sync();
    return settings.status() == QSettings::NoError;
#else
    DeleteSecretRequest dsr;
    dsr.setManager(secretManager);
    dsr.setIdentifier(toIdentifier(name));
    dsr.setUserInteractionMode(SecretManager::SystemInteraction);
    dsr.startRequest();
    dsr.waitForFinished();

    return isResultValid(dsr);
#endif
}

QString Secrets::getData(const QString &name)
{
#ifdef QT_DEBUG
    return insecureEmulatorSecrets().value(name).toString();
#else
    auto secret = getSecret(name);
    if (!isSecretValid(secret)) {
        return QString();
    }

    return QString::fromUtf8(secret.data());
#endif
}

bool Secrets::createCollection()
{
    CreateCollectionRequest ccr;
    ccr.setManager(secretManager);
    ccr.setCollectionName(collectionName);
    ccr.setAccessControlMode(SecretManager::OwnerOnlyMode);
    ccr.setCollectionLockType(CreateCollectionRequest::DeviceLock);
    ccr.setDeviceLockUnlockSemantic(SecretManager::DeviceLockKeepUnlocked);
    ccr.setStoragePluginName(SecretManager::DefaultEncryptedStoragePluginName);
    ccr.setEncryptionPluginName(SecretManager::DefaultEncryptedStoragePluginName);
    ccr.startRequest();
    ccr.waitForFinished();

    auto success = isResultValid(ccr);
    hasBitsailorCollection = success;

    return success;
}

Secret::Identifier Secrets::toIdentifier(const QString &name)
{
    return Secret::Identifier(name, collectionName, SecretManager::DefaultEncryptedStoragePluginName);
}
