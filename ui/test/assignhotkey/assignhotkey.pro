include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = assignhotkey_test

QT      += testlib gui script
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += ../../../plugins/interfaces
INCLUDEPATH += ../../../engine/src
INCLUDEPATH += ../../src
DEPENDPATH  += ../../src

QMAKE_LIBDIR += ../../../engine/src
QMAKE_LIBDIR += ../../src
LIBS        += -lqlcplusengine -lqlcplusui

# Test sources
SOURCES += assignhotkey_test.cpp
HEADERS += assignhotkey_test.h
