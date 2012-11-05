include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = vcxypad_test

QT      += testlib xml gui script

INCLUDEPATH += ../../../plugins/interfaces
INCLUDEPATH += ../../../engine/src
INCLUDEPATH += ../../src
DEPENDPATH  += ../../src

QMAKE_LIBDIR += ../../../engine/src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine -lqlcui

# Test sources
SOURCES += vcxypad_test.cpp
HEADERS += vcxypad_test.h
