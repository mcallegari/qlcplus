include(../../../variables.pri)
include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = rgbalgorithm_test

QT      += testlib script
CONFIG  -= app_bundle

DEPENDPATH   += ../../src
INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../mastertimer
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcplusengine

SOURCES += rgbalgorithm_test.cpp
HEADERS += rgbalgorithm_test.h ../common/resource_paths.h

