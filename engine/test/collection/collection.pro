include(../../../variables.pri)
include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = collection_test

QT      += testlib
CONFIG  -= app_bundle

DEPENDPATH   += ../../src
INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
INCLUDEPATH  += ../mastertimer
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcplusengine

SOURCES += collection_test.cpp ../mastertimer/mastertimer_stub.cpp
HEADERS += collection_test.h ../mastertimer/mastertimer_stub.h
