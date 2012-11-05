include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = addfixture_test

QT      += testlib gui xml script

INCLUDEPATH += ../../src
INCLUDEPATH += ../../../engine/src
DEPENDPATH  += ../../src

QMAKE_LIBDIR += ../../src
QMAKE_LIBDIR += ../../../engine/src
LIBS         += -lqlcui -lqlcengine

# Test sources
SOURCES += addfixture_test.cpp
HEADERS += addfixture_test.h
