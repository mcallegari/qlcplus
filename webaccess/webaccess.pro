include(../variables.pri)
include(../coverage.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = qlcpluswebaccess

CONFIG += qt
QT     += core gui script
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# Engine
INCLUDEPATH     += ../engine/src ../engine/src/audio ../ui/src
DEPENDPATH      += ../engine/src ../ui/src
QMAKE_LIBDIR    += ../engine/src ../ui/src
DEFINES         += USE_WEBSOCKET NO_SSL

LIBS += -lqlcplusengine -lqlcplusui

win32:LIBS  += -lws2_32
win32:QMAKE_LFLAGS += -shared
win32:INCLUDEPATH += ./

HEADERS += mongoose.h \
           commonjscss.h \
           webaccess.h

SOURCES += mongoose.c \
           webaccess.cpp
    
macx {
    # This must be after "TARGET = " and before target installation so that
    # install_name_tool can be run before target installation
    include(../macx/nametool.pri)
}

target.path = $$INSTALLROOT/$$LIBSDIR
INSTALLS   += target
