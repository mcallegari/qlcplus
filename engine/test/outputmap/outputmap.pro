include(../../../variables.pri)
include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = outputmap_test

QT      += testlib xml script
CONFIG  -= app_bundle

DEPENDPATH   += ../../src
INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
INCLUDEPATH  += ../outputpluginstub
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcplusengine

SOURCES += outputmap_test.cpp
HEADERS += outputmap_test.h
