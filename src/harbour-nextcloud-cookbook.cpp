#include <QQuickView>
#include <QScopedPointer>
#include <QGuiApplication>
#include <QtQml>
#include <QQmlEngine>

#include <sailfishapp.h>
#include "secrets.h"

constexpr auto TRANSLATION_INSTALL_DIR = "/usr/share/harbour-nextcloud-cookbook/translations";

int main(int argc, char *argv[])
{
    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
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
    v->rootContext()->setContextProperty("secrets", new Secrets(app.data()));
    // end custom deps

    v->setSource(SailfishApp::pathToMainQml());
    v->show();

    return app->exec();
}
