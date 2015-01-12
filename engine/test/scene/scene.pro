include(../../../variables.pri)
include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = scene_test

QT      += testlib xml script
CONFIG  -= app_bundle

DEPENDPATH   += ../../src
INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../mastertimer
INCLUDEPATH  += ../iopluginstub
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcplusengine

SOURCES += scene_test.cpp ../mastertimer/mastertimer_stub.cpp
HEADERS += scene_test.h ../mastertimer/mastertimer_stub.h ../common/resource_paths.h

