#ifndef SECRETS_H
#define SECRETS_H

#include <QObject>
#include <QString>

#ifndef QT_DEBUG
#include <Sailfish/Secrets/secretmanager.h>
#include <Sailfish/Secrets/request.h>
#include <Sailfish/Secrets/secret.h>

using Sailfish::Secrets::SecretManager;
using Sailfish::Secrets::Request;
using Sailfish::Secrets::Secret;
#endif

class Secrets : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
    Q_PROPERTY(QString nextcloudUrl READ nextcloudUrl WRITE setNextcloudUrl NOTIFY nextcloudUrlChanged)
    Q_PROPERTY(bool keepScreenOn READ keepScreenOn WRITE setKeepScreenOn NOTIFY keepScreenOnChanged)
public:
    explicit Secrets(QObject *parent = nullptr);

    QString username();
    QString password();
    QString nextcloudUrl();
    bool keepScreenOn();

    void setUsername(const QString &username);
    void setPassword(const QString &password);
    void setNextcloudUrl(const QString &url);
    void setKeepScreenOn(bool on);

    bool clearAllSecrets();

signals:
    void usernameChanged();
    void passwordChanged();
    void nextcloudUrlChanged();
    void keepScreenOnChanged();

private:
    bool storeData(const QString &name, const QString &data);
    QString getData(const QString &name);
    bool deleteSecret(const QString &name);
#ifndef QT_DEBUG
    static const QString collectionName;
    SecretManager* secretManager = new SecretManager(this);
    bool hasBitsailorCollection = false;

    bool isResultValid(const Request &request);
    bool isSecretValid(const Secret &secret);
    Secret getSecret(const QString &name);
    bool createCollection();
    Secret::Identifier toIdentifier(const QString &name);
#endif
};

#endif // SECRETS_H
