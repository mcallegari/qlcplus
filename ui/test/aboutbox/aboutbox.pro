include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = aboutbox_test

QT      += testlib gui xml script

INCLUDEPATH += ../../src
INCLUDEPATH += ../../../engine/src
DEPENDPATH  += ../../src

QMAKE_LIBDIR += ../../src
QMAKE_LIBDIR += ../../../engine/src
LIBS         += -lqlcplusui -lqlcplusengine

QMAKE_LIBDIR += ../../../webaccess
LIBS         += -lqlcpluswebaccess

# Test sources
SOURCES += aboutbox_test.cpp
HEADERS += aboutbox_test.h
