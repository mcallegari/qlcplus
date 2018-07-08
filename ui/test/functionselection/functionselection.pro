include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = functionselection_test

QT      += testlib gui script
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += ../../src
INCLUDEPATH += ../../../engine/src
DEPENDPATH  += ../../src

QMAKE_LIBDIR += ../../src
QMAKE_LIBDIR += ../../../engine/src
LIBS         += -lqlcplusui -lqlcplusengine

# Test sources
SOURCES += functionselection_test.cpp
HEADERS += functionselection_test.h
