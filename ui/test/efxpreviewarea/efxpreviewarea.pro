include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = efxpreviewarea_test

QT      += testlib gui xml script

INCLUDEPATH += ../../src
INCLUDEPATH += ../../../engine/src
DEPENDPATH  += ../../src

QMAKE_LIBDIR += ../../src
QMAKE_LIBDIR += ../../../engine/src
LIBS         += -lqlcui -lqlcengine

# Test sources
SOURCES += efxpreviewarea_test.cpp
HEADERS += efxpreviewarea_test.h
