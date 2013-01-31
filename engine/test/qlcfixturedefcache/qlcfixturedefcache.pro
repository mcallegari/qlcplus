include(../../../variables.pri)
include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = qlcfixturedefcache_test

QT      += testlib xml script
CONFIG  -= app_bundle

DEPENDPATH   += ../../src
INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcplusengine

SOURCES += qlcfixturedefcache_test.cpp
HEADERS += qlcfixturedefcache_test.h
