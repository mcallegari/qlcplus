include(../variables.pri)
include(../coverage.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = qlcpluswebaccess

CONFIG += qt
QT     += core xml gui script

# Engine
INCLUDEPATH     += ../engine/src ../engine/src/audio ../ui/src
DEPENDPATH      += ../../engine/src
DEFINES         += USE_WEBSOCKET

win32:QMAKE_LFLAGS += -shared

HEADERS += mongoose.h \
    webaccess.h

SOURCES += mongoose.c \
    webaccess.cpp

target.path = $$INSTALLROOT/$$LIBSDIR
INSTALLS   += target
