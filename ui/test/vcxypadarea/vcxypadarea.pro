include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = vcxypadarea_test

QT      += testlib gui script
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += ../../../plugins/interfaces
INCLUDEPATH += ../../../engine/src
INCLUDEPATH += ../../src ../../src/virtualconsole
DEPENDPATH  += ../../src

QMAKE_LIBDIR += ../../../engine/src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcplusengine -lqlcplusui

SOURCES += vcxypadarea_test.cpp
HEADERS += vcxypadarea_test.h
