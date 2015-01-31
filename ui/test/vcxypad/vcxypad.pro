include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = vcxypad_test

QT      += testlib xml gui script
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += ../../../plugins/interfaces
INCLUDEPATH += ../../../engine/src
INCLUDEPATH += ../../src ../../src/virtualconsole
DEPENDPATH  += ../../src

QMAKE_LIBDIR += ../../../engine/src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcplusengine -lqlcplusui

# Test sources
SOURCES += vcxypad_test.cpp
HEADERS += vcxypad_test.h ../common/resource_paths.h
