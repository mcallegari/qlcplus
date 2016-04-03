include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = monitorfixtureitem_test

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
SOURCES += monitorfixtureitem_test.cpp
HEADERS += monitorfixtureitem_test.h
