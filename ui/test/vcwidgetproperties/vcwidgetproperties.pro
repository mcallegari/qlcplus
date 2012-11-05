include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = vcwidgetproperties_test

QT += testlib gui xml script

INCLUDEPATH += ../../../plugins/interfaces
INCLUDEPATH += ../../src
DEPENDPATH  += ../../src

QMAKE_LIBDIR += ../../src
QMAKE_LIBDIR += ../../../engine/src
LIBS         += -lqlcui -lqlcengine

# Test sources
SOURCES += vcwidgetproperties_test.cpp
HEADERS += vcwidgetproperties_test.h
