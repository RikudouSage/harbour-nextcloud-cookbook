TARGET = harbour-nextcloud-cookbook
CONFIG += sailfishapp c++20
PKGCONFIG += sailfishsecrets sailfishcrypto
QT += concurrent

GO_LIBDIR = /usr/share/$$TARGET/lib
INCLUDEPATH += $$PWD/core
LIBS += -L$$PWD/core -lcookbook
QMAKE_RPATHDIR += $$GO_LIBDIR
libcookbook.path = $$GO_LIBDIR
libcookbook.files = $$PWD/core/libcookbook.so
INSTALLS += libcookbook

SOURCES += src/harbour-nextcloud-cookbook.cpp \
    src/core.cpp \
    src/secrets.cpp

DISTFILES += qml/harbour-nextcloud-cookbook.qml \
    qml/components/SafePage.qml \
    qml/components/StandardDialog.qml \
    qml/components/StandardLabel.qml \
    qml/components/StandardPage.qml \
    qml/cover/CoverPage.qml \
    qml/pages/CheckPage.qml \
    qml/pages/LoginPage.qml \
    rpm/harbour-nextcloud-cookbook.changes.in \
    rpm/harbour-nextcloud-cookbook.changes.run.in \
    rpm/harbour-nextcloud-cookbook.spec \
    translations/*.ts \
    harbour-nextcloud-cookbook.desktop

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

# to disable building translations every time, comment out the
# following CONFIG line
CONFIG += sailfishapp_i18n sailfishapp_i18n_idbased

# German translation is enabled as an example. If you aren't
# planning to localize your app, remember to comment out the
# following TRANSLATIONS line. And also do not forget to
# modify the localized app name in the the .desktop file.
TRANSLATIONS += translations/harbour-nextcloud-cookbook-en.ts \
                translations/harbour-nextcloud-cookbook-cs.ts

HEADERS += \
    core/libcookbook.h \
    src/core.h \
    src/secrets.h
