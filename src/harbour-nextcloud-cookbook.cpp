#include <QQuickView>
#include <QScopedPointer>
#include <QGuiApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QStandardPaths>
#include <QTextStream>
#include <QtQml>
#include <QQmlEngine>

#include <cstdio>

#include <sailfishapp.h>
#include "secrets.h"
#include "core.h"

constexpr auto TRANSLATION_INSTALL_DIR = "/usr/share/harbour-nextcloud-cookbook/translations";

namespace {

QMutex messageHandlerMutex;
QtMessageHandler defaultMessageHandler = nullptr;

QString messageTypeName(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:
        return QStringLiteral("debug");
    case QtInfoMsg:
        return QStringLiteral("info");
    case QtWarningMsg:
        return QStringLiteral("warning");
    case QtCriticalMsg:
        return QStringLiteral("critical");
    case QtFatalMsg:
        return QStringLiteral("fatal");
    }

    return QStringLiteral("unknown");
}

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    if (defaultMessageHandler != nullptr) {
        defaultMessageHandler(type, context, message);
    } else {
        fprintf(stderr, "%s\n", qPrintable(message));
        fflush(stderr);
    }

    if (type != QtWarningMsg) {
        return;
    }

    const QMutexLocker lock(&messageHandlerMutex);
    const auto logDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (logDir.isEmpty() || !QDir().mkpath(logDir)) {
        return;
    }

    QFile file(QDir(logDir).filePath(QStringLiteral("warnings.log")));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        return;
    }

    QTextStream stream(&file);
    stream << QDateTime::currentDateTimeUtc().toString(Qt::ISODate)
           << " [" << messageTypeName(type) << "] " << message;

    if (context.file != nullptr) {
        stream << " (" << context.file << ':' << context.line << ')';
    }

    stream << '\n';
}

}

int main(int argc, char *argv[])
{
    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
    defaultMessageHandler = qInstallMessageHandler(messageHandler);
    QScopedPointer<QQuickView> v(SailfishApp::createView());

    QTranslator *defaultLang = new QTranslator(app.data());
    if (!defaultLang->load("harbour-nextcloud-cookbook-en", TRANSLATION_INSTALL_DIR)) {
        qWarning() << "Could not load English translation file!";
    }
    QCoreApplication::installTranslator(defaultLang);

    QTranslator *translator = new QTranslator(app.data());
    if (!translator->load(QLocale(QLocale::system().name()), "harbour-nextcloud-cookbook", "-", TRANSLATION_INSTALL_DIR)) {
        qWarning() << "Could not load translations for" << QLocale::system().name();
    }
    QCoreApplication::installTranslator(translator);

    // custom deps
    auto secrets = new Secrets(app.data());
    auto core = new Core(secrets, app.data());

    v->rootContext()->setContextProperty("secrets", secrets);
    v->rootContext()->setContextProperty("core", core);
    // end custom deps

    v->setSource(SailfishApp::pathToMainQml());
    v->show();

    return app->exec();
}
