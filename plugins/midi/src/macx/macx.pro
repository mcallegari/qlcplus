include(../../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = midiplugin

CONFIG      += plugin
INCLUDEPATH += ../common
LIBS        += -framework CoreMIDI -framework CoreFoundation

include(../common/common.pri)

HEADERS += coremidiinputdevice.h \
           coremidioutputdevice.h \
           coremidienumeratorprivate.h

SOURCES += coremidiinputdevice.cpp \
           coremidioutputdevice.cpp \
           coremidienumerator.cpp
