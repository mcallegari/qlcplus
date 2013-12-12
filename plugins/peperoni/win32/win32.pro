include (../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = peperoni

INCLUDEPATH += peperoni
INCLUDEPATH += ../../interfaces
DEPENDPATH  += peperoni
CONFIG      += plugin
QMAKE_LFLAGS += -shared
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# Headers
HEADERS += peperoni/usbdmx-dynamic.h \
           peperonidevice.h \
           peperoni.h

# Sources
SOURCES += peperoni/usbdmx-dynamic.cpp \
           peperonidevice.cpp \
           peperoni.cpp

HEADERS += ../../interfaces/qlcioplugin.h

# Installation
target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target
