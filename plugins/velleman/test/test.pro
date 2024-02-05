include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = velleman_test

QT      += core testlib
QT      -= gui
LIBS    += -L../src -lvelleman

INCLUDEPATH += ../../interfaces
INCLUDEPATH += ../src
DEPENDPATH  += ../src

# Test sources
HEADERS += velleman_test.h ../../interfaces/qlcioplugin.h
SOURCES += velleman_test.cpp  ../src/velleman_mock.cpp ../../interfaces/qlcioplugin.cpp
