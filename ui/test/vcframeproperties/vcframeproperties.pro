include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = vcframeproperties_test

QT      += testlib gui script
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

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
