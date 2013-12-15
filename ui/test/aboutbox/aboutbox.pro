include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = aboutbox_test

QT      += testlib gui xml script
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += ../../src
INCLUDEPATH += $${OUT_PWD}/../../src/
INCLUDEPATH += ../../../engine/src

DEPENDPATH  += ../../src

QMAKE_LIBDIR += ../../src
QMAKE_LIBDIR += ../../../engine/src
LIBS         += -lqlcplusui -lqlcplusengine

# Test sources
SOURCES += aboutbox_test.cpp
HEADERS += aboutbox_test.h
