include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = monitorfixture_test

QT      += testlib xml gui script

INCLUDEPATH += ../../../plugins/interfaces
INCLUDEPATH += ../../../engine/src
INCLUDEPATH += ../../src
DEPENDPATH  += ../../src

QMAKE_LIBDIR += ../../../engine/src
QMAKE_LIBDIR += ../../src
LIBS        += -lqlcengine -lqlcui

# Test sources
SOURCES += monitorfixture_test.cpp
HEADERS += monitorfixture_test.h
