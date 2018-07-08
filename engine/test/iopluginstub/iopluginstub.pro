include(../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = iopluginstub

CONFIG      += plugin
INCLUDEPATH += ../../../plugins/interfaces
DEPENDPATH  += ../../../plugins/interfaces
win32:QMAKE_LFLAGS += -shared

HEADERS += ../../../plugins/interfaces/qlcioplugin.h
HEADERS += iopluginstub.h

SOURCES += ../../../plugins/interfaces/qlcioplugin.cpp
SOURCES += iopluginstub.cpp
