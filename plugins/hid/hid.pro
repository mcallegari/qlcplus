include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = hid

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += ../interfaces
CONFIG      += plugin

HEADERS += ../interfaces/qlcioplugin.h
HEADERS += configurehid.h \
           hiddevice.h \
           hideventdevice.h \
           hid.h \
           hidjsdevice.h \
           hidfx5device.h \
           hidpoller.h

FORMS += configurehid.ui

SOURCES += configurehid.cpp \
           hiddevice.cpp \
           hideventdevice.cpp \
           hid.cpp \
           hidjsdevice.cpp \
           hidfx5device.cpp \
           hidpoller.cpp

win32:SOURCES += win32/hidapi.cpp

unix:CONFIG    += link_pkgconfig
unix:PKGCONFIG += libusb
unix:!macx:SOURCES += linux/hidapi.cpp

macx:SOURCES += macx/hidapi.cpp

TRANSLATIONS += HID_fi_FI.ts
TRANSLATIONS += HID_de_DE.ts
TRANSLATIONS += HID_es_ES.ts
TRANSLATIONS += HID_fr_FR.ts
TRANSLATIONS += HID_it_IT.ts
TRANSLATIONS += HID_nl_NL.ts
TRANSLATIONS += HID_cz_CZ.ts

target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target
