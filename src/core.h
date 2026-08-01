#ifndef CORE_H
#define CORE_H

#include <QObject>

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

signals:
    void credentialsValidated(bool success);
    void credentialsValidated(bool success, const QString &url, const QString &username, const QString &password);
    void initialized(bool success);

private:
    Secrets *secrets;
    bool valid;

    ClientHandle client = 0;
    ContextHandle ctx = 0;

private:
    void initialize();
    void cleanup();
    const QString getLastError() const;
};

#endif // CORE_H
