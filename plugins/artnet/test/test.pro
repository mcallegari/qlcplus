include(../../../variables.pri)
include(../../../coverage.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = artnet_test

QT      += core testlib network
QT      -= gui
LIBS    += -L../src -lartnet

INCLUDEPATH += ../../interfaces
INCLUDEPATH += ../src
DEPENDPATH  += ../src

# Test sources
HEADERS += artnet_test.h ../../interfaces/qlcioplugin.h
SOURCES += artnet_test.cpp  ../src/artnetpacketizer.cpp ../../interfaces/qlcioplugin.cpp
