include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = hidplugin

QT += widgets

INCLUDEPATH += ../interfaces
CONFIG      += plugin

unix:!macx:INCLUDEPATH += linux
macx:INCLUDEPATH += macx

win32:LIBS += -lsetupapi -lwinmm -lhid
macx:LIBS += -framework IOKit -framework CoreFoundation

HEADERS += ../interfaces/qlcioplugin.h

HEADERS += configurehid.h \
           hiddevice.h \
           hidplugin.h \
           hidjsdevice.h \
           hiddmxdevice.h

unix:!macx:HEADERS += linux/hidlinuxjoystick.h
win32:HEADERS += win32/hidwindowsjoystick.h
macx:HEADERS += macx/hidosxjoystick.h

FORMS += configurehid.ui

SOURCES += ../interfaces/qlcioplugin.cpp

SOURCES += configurehid.cpp \
           hiddevice.cpp \
           hidplugin.cpp \
           hidjsdevice.cpp \
           hiddmxdevice.cpp

unix:!macx:SOURCES += linux/hidapi.cpp linux/hidlinuxjoystick.cpp
win32:SOURCES += win32/hidapi.cpp win32/hidwindowsjoystick.cpp
macx:SOURCES += macx/hidapi.cpp macx/hidosxjoystick.cpp

unix:!macx {
    # Rules to make FX5 DMX devices readable & writable by normal users
    udev.path  = $$UDEVRULESDIR
    udev.files = linux/z65-fx5-hid.rules
    INSTALLS  += udev

    metainfo.path   = $$METAINFODIR
    metainfo.files += linux/org.qlcplus.QLCPlus.hid.metainfo.xml
    INSTALLS       += metainfo
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
macx:include(../../platforms/macos/nametool.pri)

target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target
