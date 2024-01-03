include(../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = udmx

QT += widgets
macx:QT_CONFIG -= no-pkg-config

CONFIG      += plugin
INCLUDEPATH += ../../interfaces
DEPENDPATH  += ../../interfaces
CONFIG      += link_pkgconfig
PKGCONFIG   += libusb-1.0
win32:QMAKE_LFLAGS += -shared

HEADERS += ../../interfaces/qlcioplugin.h
HEADERS += udmxdevice.h \
           udmx.h

SOURCES += ../../interfaces/qlcioplugin.cpp
SOURCES += udmxdevice.cpp \
           udmx.cpp

TRANSLATIONS += uDMX_fi_FI.ts
TRANSLATIONS += uDMX_de_DE.ts
TRANSLATIONS += uDMX_es_ES.ts
TRANSLATIONS += uDMX_fr_FR.ts
TRANSLATIONS += uDMX_it_IT.ts
TRANSLATIONS += uDMX_nl_NL.ts
TRANSLATIONS += uDMX_cz_CZ.ts
TRANSLATIONS += uDMX_pt_BR.ts
TRANSLATIONS += uDMX_ca_ES.ts
TRANSLATIONS += uDMX_ja_JP.ts

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
macx {
    include(../../../platforms/macos/nametool.pri)
    nametool.commands += $$pkgConfigNametool(libusb-1.0, libusb-1.0.0.dylib)
}

# Installation
target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target

# UDEV rule to make uDMX USB device readable & writable for users in Linux
unix:!macx {
    udev.path  = $$UDEVRULESDIR
    udev.files = z65-anyma-udmx.rules
    INSTALLS  += udev

    metainfo.path   = $$METAINFODIR
    metainfo.files += org.qlcplus.QLCPlus.udmx.metainfo.xml
    INSTALLS       += metainfo
}
