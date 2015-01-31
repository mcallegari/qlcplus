include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = addfixture_test

QT      += testlib gui xml script
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += ../../src
INCLUDEPATH += ../../../engine/src
DEPENDPATH  += ../../src

QMAKE_LIBDIR += ../../src
QMAKE_LIBDIR += ../../../engine/src
LIBS         += -lqlcplusui -lqlcplusengine

# Test sources
SOURCES += addfixture_test.cpp
HEADERS += addfixture_test.h ../common/resource_paths.h
