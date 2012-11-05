include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = vcwidget_test

QT      += testlib xml gui script

INCLUDEPATH += ../../../plugins/interfaces
INCLUDEPATH += ../../../engine/src
INCLUDEPATH += ../../src
DEPENDPATH  += ../../src

QMAKE_LIBDIR += ../../../engine/src
QMAKE_LIBDIR += ../../src
LIBS        += -lqlcengine -lqlcui

# Test sources
SOURCES += vcwidget_test.cpp stubwidget.cpp
HEADERS += vcwidget_test.h stubwidget.h
