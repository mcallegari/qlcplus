include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = spi

INCLUDEPATH += ../interfaces
INCLUDEPATH += /usr/include
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

HEADERS += spiplugin.h
SOURCES += spiplugin.cpp
HEADERS += ../interfaces/qlcioplugin.h
