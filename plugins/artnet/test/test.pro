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

HEADERS += ../../interfaces/qlcioplugin.h \
		   ../../interfaces/rdmprotocol.h

SOURCES += ../../interfaces/qlcioplugin.cpp \
		   ../../interfaces/rdmprotocol.cpp

# Test sources
HEADERS += artnet_test.h 

SOURCES += artnet_test.cpp \
           ../src/artnetpacketizer.cpp 
