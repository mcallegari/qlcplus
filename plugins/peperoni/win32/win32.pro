include (../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = peperoni

INCLUDEPATH += peperoni
INCLUDEPATH += ../../interfaces
DEPENDPATH  += peperoni
CONFIG      += plugin
QMAKE_LFLAGS += -shared
QT          += widgets

# Headers
HEADERS += ../../interfaces/qlcioplugin.h
HEADERS += peperoni/usbdmx-dynamic.h \
           peperonidevice.h \
           peperoni.h

# Sources
SOURCES += ../../interfaces/qlcioplugin.cpp
SOURCES += peperoni/usbdmx-dynamic.cpp \
           peperonidevice.cpp \
           peperoni.cpp

# Installation
target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target
