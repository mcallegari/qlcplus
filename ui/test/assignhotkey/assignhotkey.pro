include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = assignhotkey_test

QT      += testlib xml gui script

INCLUDEPATH += ../../../plugins/interfaces
INCLUDEPATH += ../../../engine/src
INCLUDEPATH += ../../src
DEPENDPATH  += ../../src

QMAKE_LIBDIR += ../../../engine/src
QMAKE_LIBDIR += ../../src
LIBS        += -lqlcengine -lqlcui

# Test sources
SOURCES += assignhotkey_test.cpp
HEADERS += assignhotkey_test.h
