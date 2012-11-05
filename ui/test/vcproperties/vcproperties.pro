include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = vcproperties_test

QT      += testlib xml gui script

INCLUDEPATH += ../../../plugins/interfaces
INCLUDEPATH += ../../../engine/src
INCLUDEPATH += ../../src
DEPENDPATH  += ../../src

QMAKE_LIBDIR += ../../../engine/src
QMAKE_LIBDIR += ../../src
LIBS        += -lqlcengine -lqlcui

# Test sources
SOURCES += vcproperties_test.cpp
HEADERS += vcproperties_test.h
