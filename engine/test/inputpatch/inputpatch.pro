include(../../../variables.pri)
include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = inputpatch_test

QT      += testlib
CONFIG  -= app_bundle

DEPENDPATH   += ../../src
INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
INCLUDEPATH  += ../iopluginstub
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcplusengine

SOURCES += inputpatch_test.cpp
HEADERS += inputpatch_test.h
