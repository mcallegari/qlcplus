include(../../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = midiplugin

CONFIG      += plugin
INCLUDEPATH += ../common
LIBS        += -lwinmm
QMAKE_LFLAGS += -shared

include(../common/common.pri)

HEADERS += win32midiinputdevice.h \
           win32midioutputdevice.h \
           win32midienumeratorprivate.h

SOURCES += win32midiinputdevice.cpp \
           win32midioutputdevice.cpp \
           win32midienumerator.cpp
