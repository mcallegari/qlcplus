include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = spi

QT += widgets

INCLUDEPATH += ../interfaces
INCLUDEPATH += $SYSROOT/usr/include
CONFIG      += plugin

# Rules to make SPI devices readable & writable by normal users
udev.path  = $$UDEVRULESDIR
udev.files = z65-spi.rules
INSTALLS  += udev

metainfo.path   = $$METAINFODIR
metainfo.files += org.qlcplus.QLCPlus.spi.metainfo.xml
INSTALLS       += metainfo

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

HEADERS += ../interfaces/qlcioplugin.h
HEADERS += spiplugin.h \
           spiconfiguration.h \
           spioutthread.h

SOURCES += ../interfaces/qlcioplugin.cpp
SOURCES += spiplugin.cpp \
           spiconfiguration.cpp \
           spioutthread.cpp

FORMS += spiconfiguration.ui
