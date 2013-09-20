include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = vcxypadarea_test

QT      += testlib xml gui script

INCLUDEPATH += ../../../plugins/interfaces
INCLUDEPATH += ../../../engine/src
INCLUDEPATH += ../../src
DEPENDPATH  += ../../src

QMAKE_LIBDIR += ../../../engine/src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcplusengine -lqlcplusui

QMAKE_LIBDIR += ../../../webaccess
LIBS         += -lqlcpluswebaccess

SOURCES += vcxypadarea_test.cpp
HEADERS += vcxypadarea_test.h
