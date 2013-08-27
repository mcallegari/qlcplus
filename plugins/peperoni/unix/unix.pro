include(../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = peperoni

QT          += core gui
CONFIG      += plugin
CONFIG      += link_pkgconfig
PKGCONFIG   += libusb
INCLUDEPATH += ../../interfaces

HEADERS += peperonidevice.h \
           peperoni.h

SOURCES += peperonidevice.cpp \
           peperoni.cpp

HEADERS += ../../interfaces/qlcioplugin.h

TRANSLATIONS += Peperoni_fi_FI.ts
TRANSLATIONS += Peperoni_de_DE.ts
TRANSLATIONS += Peperoni_es_ES.ts
TRANSLATIONS += Peperoni_fr_FR.ts
TRANSLATIONS += Peperoni_it_IT.ts
TRANSLATIONS += Peperoni_nl_NL.ts

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
macx:include(../../../macx/nametool.pri)

# Installation
target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target

# UDEV rule to make Peperoni USB devices readable & writable for users in Linux
udev.path  = /etc/udev/rules.d
udev.files = z65-peperoni.rules
!macx:INSTALLS  += udev
