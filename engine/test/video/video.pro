include(../../../variables.pri)
include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = video_test

QT      += testlib
CONFIG  -= app_bundle

DEPENDPATH   += ../../src
INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
INCLUDEPATH  += ../mastertimer
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcplusengine

SOURCES += video_test.cpp ../mastertimer/mastertimer_stub.cpp
HEADERS += video_test.h ../mastertimer/mastertimer_stub.h
