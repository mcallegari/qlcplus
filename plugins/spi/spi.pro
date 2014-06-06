include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = spi

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += ../interfaces
INCLUDEPATH += $SYSROOT/usr/include
CONFIG      += plugin

# Rules to make SPI devices readable & writable by normal users
udev.path  = /etc/udev/rules.d
udev.files = z65-spi.rules
INSTALLS  += udev

target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target

TRANSLATIONS += SPI_de_DE.ts
TRANSLATIONS += SPI_es_ES.ts
TRANSLATIONS += SPI_fi_FI.ts
TRANSLATIONS += SPI_fr_FR.ts
TRANSLATIONS += SPI_it_IT.ts
TRANSLATIONS += SPI_nl_NL.ts
TRANSLATIONS += SPI_cz_CZ.ts
TRANSLATIONS += SPI_pt_BR.ts
TRANSLATIONS += SPI_ca_ES.ts
TRANSLATIONS += SPI_ja_JP.ts

HEADERS += spiplugin.h spiconfiguration.h \
    spioutthread.h
SOURCES += spiplugin.cpp spiconfiguration.cpp \
    spioutthread.cpp
FORMS += spiconfiguration.ui
HEADERS += ../interfaces/qlcioplugin.h
