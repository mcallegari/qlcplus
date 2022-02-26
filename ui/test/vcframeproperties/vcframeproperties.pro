include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = vcframeproperties_test

QT      += testlib gui script widgets

INCLUDEPATH += ../../../plugins/interfaces
INCLUDEPATH += ../../../engine/src
INCLUDEPATH += ../../src ../../src/virtualconsole
DEPENDPATH  += ../../src

QMAKE_LIBDIR += ../../../engine/src
QMAKE_LIBDIR += ../../src
LIBS        += -lqlcplusengine -lqlcplusui

# Test sources
SOURCES += vcframeproperties_test.cpp
HEADERS += vcframeproperties_test.h
