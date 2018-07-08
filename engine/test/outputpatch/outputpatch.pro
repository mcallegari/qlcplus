include(../../../variables.pri)
include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = outputpatch_test

QT      += testlib
CONFIG  -= app_bundle

DEPENDPATH   += ../../src
INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
INCLUDEPATH  += ../iopluginstub
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcplusengine

SOURCES += outputpatch_test.cpp
HEADERS += outputpatch_test.h
