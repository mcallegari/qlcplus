include(../../../variables.pri)
include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = chaserrunner_test

QT      += testlib xml script
CONFIG  -= app_bundle

DEPENDPATH   += ../../src
INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../mastertimer
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcplusengine

SOURCES += chaserrunner_test.cpp ../mastertimer/mastertimer_stub.cpp
HEADERS += chaserrunner_test.h ../mastertimer/mastertimer_stub.h ../common/resource_paths.h

