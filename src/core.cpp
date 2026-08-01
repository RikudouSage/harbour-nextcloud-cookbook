#include "core.h"
#include <QDebug>

#include <QtConcurrent>

Core::Core(Secrets *secrets, QObject *parent)
    : QObject(parent), secrets(secrets)
{
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
