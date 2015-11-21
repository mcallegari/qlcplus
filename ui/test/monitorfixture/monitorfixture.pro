include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = monitorfixture_test

QT      += testlib gui script
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += ../../../plugins/interfaces
INCLUDEPATH += ../../../engine/src
INCLUDEPATH += ../../src ../../src/monitor
DEPENDPATH  += ../../src

QMAKE_LIBDIR += ../../../engine/src
QMAKE_LIBDIR += ../../src
LIBS        += -lqlcplusengine -lqlcplusui

# Test sources
SOURCES += monitorfixture_test.cpp
HEADERS += monitorfixture_test.h
