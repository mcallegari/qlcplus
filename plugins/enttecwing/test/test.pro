include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = enttecwing_test

QT     += core gui network testlib widgets

INCLUDEPATH += ../../interfaces
INCLUDEPATH += ../src
LIBS        += -L../src -lenttecwing

SOURCES += playbackwing_test.cpp \
           programwing_test.cpp \
           shortcutwing_test.cpp \
           wing_test.cpp \
           main.cpp

HEADERS += playbackwing_test.h \
           programwing_test.h \
           shortcutwing_test.h \
           wing_test.h
