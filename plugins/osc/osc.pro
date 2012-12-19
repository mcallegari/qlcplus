include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = osc

CONFIG      += plugin
CONFIG      += link_pkgconfig
INCLUDEPATH += ../interfaces
DEPENDPATH  += ../interfaces

PKGCONFIG += liblo
win32:QMAKE_LFLAGS += -shared

target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target

#TRANSLATIONS += OSC_de_DE.ts
#TRANSLATIONS += OSC_es_ES.ts
#TRANSLATIONS += OSC_fi_FI.ts
#TRANSLATIONS += OSC_fr_FR.ts
#TRANSLATIONS += OSC_it_IT.ts

HEADERS += oscplugin.h
SOURCES += oscplugin.cpp
HEADERS += ../interfaces/qlcioplugin.h
