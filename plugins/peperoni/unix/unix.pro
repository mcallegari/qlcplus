include(../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = peperoni

QT          += core gui widgets
macx:QT_CONFIG -= no-pkg-config

CONFIG      += plugin
CONFIG      += link_pkgconfig
PKGCONFIG   += libusb-1.0
INCLUDEPATH += ../../interfaces
INCLUDEPATH += ../common

HEADERS += ../../interfaces/qlcioplugin.h
HEADERS += peperonidevice.h \
           peperoni.h

SOURCES += ../../interfaces/qlcioplugin.cpp
SOURCES += peperonidevice.cpp \
           peperoni.cpp

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
macx {
    include(../../../platforms/macos/nametool.pri)
    nametool.commands += $$pkgConfigNametool(libusb-1.0, libusb-1.0.0.dylib)
}

# Installation
target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target

# UDEV rule to make Peperoni USB devices readable & writable for users in Linux
udev.path  = $$UDEVRULESDIR
udev.files = z65-peperoni.rules
!macx:INSTALLS  += udev

unix:!macx {
   metainfo.path   = $$METAINFODIR
   metainfo.files += org.qlcplus.QLCPlus.peperoni.metainfo.xml
   INSTALLS       += metainfo
}

