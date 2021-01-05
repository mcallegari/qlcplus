include(../../../variables.pri)
include(../../../coverage.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = midi_test

QT      += core testlib
QT      -= gui
LIBS    += -L../src

INCLUDEPATH += ../../interfaces
INCLUDEPATH += ../src/common
DEPENDPATH  += ../src

# Test sources
HEADERS += midi_test.h ../../interfaces/qlcioplugin.h ../src/common/midiprotocol.h
SOURCES += midi_test.cpp  ../src/common/midiprotocol.cpp ../../interfaces/qlcioplugin.cpp
