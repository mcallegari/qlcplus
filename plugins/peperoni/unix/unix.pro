include(../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = peperoni

QT          += core gui
greaterThan(QT_MAJOR_VERSION, 4) {
  QT += widgets
  macx:QT_CONFIG -= no-pkg-config
}
CONFIG      += plugin
CONFIG      += link_pkgconfig
PKGCONFIG   += libusb
INCLUDEPATH += ../../interfaces
INCLUDEPATH += ../common

HEADERS += peperonidevice.h \
           peperoni.h

SOURCES += peperonidevice.cpp \
           peperoni.cpp

HEADERS += ../../interfaces/qlcioplugin.h

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
