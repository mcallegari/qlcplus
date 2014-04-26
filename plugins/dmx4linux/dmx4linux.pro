include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = dmx4linux

INCLUDEPATH += ../interfaces
CONFIG      += plugin

target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target

TRANSLATIONS += DMX4Linux_de_DE.ts
TRANSLATIONS += DMX4Linux_es_ES.ts
TRANSLATIONS += DMX4Linux_fi_FI.ts
TRANSLATIONS += DMX4Linux_fr_FR.ts
TRANSLATIONS += DMX4Linux_it_IT.ts
TRANSLATIONS += DMX4Linux_nl_NL.ts
TRANSLATIONS += DMX4Linux_cz_CZ.ts
TRANSLATIONS += DMX4Linux_pt_BR.ts

HEADERS += dmx4linux.h
SOURCES += dmx4linux.cpp
HEADERS += ../interfaces/qlcioplugin.h
