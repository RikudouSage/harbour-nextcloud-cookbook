# NOTICE:
#
# Application name defined in TARGET has a corresponding QML filename.
# If name defined in TARGET is changed, the following needs to be done
# to match new name:
#   - corresponding QML filename must be changed
#   - desktop icon filename must be changed
#   - desktop filename must be changed
#   - icon definition filename in desktop file must be changed
#   - translation filenames have to be changed

# The name of your application
TARGET = harbour-nextcloud-cookbook

CONFIG += sailfishapp c++20
PKGCONFIG += sailfishsecrets sailfishcrypto

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
    src/core.h \
    src/secrets.h
