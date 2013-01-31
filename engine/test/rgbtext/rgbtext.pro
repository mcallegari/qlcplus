include(../../../variables.pri)
include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = rgbtext_test

QT      += testlib xml script
CONFIG  -= app_bundle

DEPENDPATH   += ../../src
INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../mastertimer
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcplusengine

SOURCES += rgbtext_test.cpp
HEADERS += rgbtext_test.h
