include(../../../variables.pri)
include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = sequence_test

QT      += testlib
CONFIG  -= app_bundle

DEPENDPATH   += ../../src
INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
INCLUDEPATH  += ../mastertimer
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcplusengine

SOURCES += sequence_test.cpp ../mastertimer/mastertimer_stub.cpp
HEADERS += sequence_test.h ../mastertimer/mastertimer_stub.h
