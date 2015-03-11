include(../../../variables.pri)
include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = palettegenerator_test

QT      += testlib xml script
CONFIG  -= app_bundle

INCLUDEPATH += ../../src
INCLUDEPATH += ../../../engine/src
DEPENDPATH  += ../../src

QMAKE_LIBDIR += ../../src
QMAKE_LIBDIR += ../../../engine/src
LIBS         += -lqlcplusui -lqlcplusengine

SOURCES += palettegenerator_test.cpp
HEADERS += palettegenerator_test.h ../common/resource_paths.h

