include(../../variables.pri)
include(../hotplugmonitor.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = test

DEPENDPATH  += ../src
INCLUDEPATH += ../src
LIBS        += -L../src -lhotplugmonitor

SOURCES += main.cpp hpmtest.cpp
HEADERS += hpmtest.h
