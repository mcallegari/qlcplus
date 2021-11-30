include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = serial-osc-bridge

## if build fails because of 'serial port', install the add-on package
## with "apt-get install libqt5serialport5-dev"
QT += serialport
QT += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += ../interfaces
CONFIG      += plugin

target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target

HEADERS += ../interfaces/qlcioplugin.h
HEADERS += serial-osc-bridge-plugin.h \
           serial-osc-bridge-configuration.h \
           serial-osc-bridge-readerthread.h \
           serial-osc-bridge-settings.h \
           serial-osc-bridge-utils.h

SOURCES += ../interfaces/qlcioplugin.cpp
SOURCES += serial-osc-bridge-plugin.cpp \
           serial-osc-bridge-configuration.cpp \
           serial-osc-bridge-readerthread.cpp \
           serial-osc-bridge-settings.cpp \
           serial-osc-bridge-utils.cpp

FORMS += serial-osc-bridge-configuration.ui
