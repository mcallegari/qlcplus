include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = efxpreviewarea_test

QT      += testlib gui script
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += ../../src
INCLUDEPATH += ../../../engine/src
DEPENDPATH  += ../../src

QMAKE_LIBDIR += ../../src
QMAKE_LIBDIR += ../../../engine/src 
LIBS         += -lqlcplusui -lqlcplusengine

# Test sources
SOURCES += efxpreviewarea_test.cpp
HEADERS += efxpreviewarea_test.h
