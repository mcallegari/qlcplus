include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = hidplugin

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += ../interfaces
CONFIG      += plugin

win32:LIBS += -lsetupapi -lwinmm
macx:LIBS += -framework IOKit -framework CoreFoundation

HEADERS += ../interfaces/qlcioplugin.h
HEADERS += configurehid.h \
           hiddevice.h \
           hidplugin.h \
           hidjsdevice.h \
           hidfx5device.h

FORMS += configurehid.ui

SOURCES += configurehid.cpp \
           hiddevice.cpp \
           hidplugin.cpp \
           hidjsdevice.cpp \
           hidfx5device.cpp

unix:!macx:SOURCES += linux/hidapi.cpp
win32:SOURCES += win32/hidapi.cpp
macx:SOURCES += macx/hidapi.cpp

unix:!macx {
    # Rules to make FX5 DMX devices readable & writable by normal users
    udev.path  = /etc/udev/rules.d
    udev.files = linux/z65-fx5-hid.rules
    INSTALLS  += udev
}

TRANSLATIONS += HID_fi_FI.ts
TRANSLATIONS += HID_de_DE.ts
TRANSLATIONS += HID_es_ES.ts
TRANSLATIONS += HID_fr_FR.ts
TRANSLATIONS += HID_it_IT.ts
TRANSLATIONS += HID_nl_NL.ts
TRANSLATIONS += HID_cz_CZ.ts
TRANSLATIONS += HID_pt_BR.ts
TRANSLATIONS += HID_ca_ES.ts
TRANSLATIONS += HID_ja_JP.ts

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
macx:include(../../macx/nametool.pri)

target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target
