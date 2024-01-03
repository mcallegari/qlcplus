include(../../../variables.pri)
include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = track_test

QT      += testlib
CONFIG  -= app_bundle

DEPENDPATH   += ../../src
INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcplusengine

SOURCES += track_test.cpp
HEADERS += track_test.h
