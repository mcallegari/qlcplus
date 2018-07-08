include(../../variables.pri)
include(../hotplugmonitor.pri)
TEMPLATE = lib
LANGUAGE = C++
TARGET   = hotplugmonitor
CONFIG  += staticlib

CONFIG(udev) {
    SOURCES   += hpmprivate-udev.cpp
    HEADERS   += hpmprivate-udev.h
    CONFIG    += link_pkgconfig
    PKGCONFIG += libudev
}

CONFIG(iokit) {
    SOURCES   += hpmprivate-iokit.cpp
    HEADERS   += hpmprivate-iokit.h
}

CONFIG(win32) {
    SOURCES += hpmprivate-win32.cpp
    HEADERS += hpmprivate-win32.h
}

# Common sources
SOURCES += hotplugmonitor.cpp
HEADERS += hotplugmonitor.h
