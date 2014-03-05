include(../../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = fx5

CONFIG      += plugin
INCLUDEPATH += ../../interfaces
DEPENDPATH  += ../../interfaces
unix:CONFIG      += link_pkgconfig

INCLUDEPATH += common
!macx:!win32 {
    # HID API Library Implementation (Linux)
    SOURCES += linux/hid.c
    INCLUDEPATH += linux
}

# HID API Library Header
HEADERS += linux/hidapi.h

# Original FX5 driver source:
HEADERS += common/fx5driver.h
SOURCES += common/fx5driver.c

# Driver - Device class abstraction layer
HEADERS += common/fx5device.h
SOURCES += common/fx5device.cpp

# High-Level FX5 Driver Class
HEADERS += common/fx5.h
SOURCES += common/fx5.cpp

HEADERS += ../../../interfaces/qlcioplugin.h

TRANSLATIONS += translations/fx5_de_DE.ts

# Installation
target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target

unix:!macx {
    # Linux UDEV rule for the FX5 USB device
    udev.path  = /etc/udev/rules.d
    udev.files = linux/50-usbdmx.rules
    INSTALLS  += udev
}
