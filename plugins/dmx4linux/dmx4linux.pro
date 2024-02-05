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
TRANSLATIONS += DMX4Linux_ca_ES.ts
TRANSLATIONS += DMX4Linux_ja_JP.ts

HEADERS += ../interfaces/qlcioplugin.h
HEADERS += dmx4linux.h

SOURCES += ../interfaces/qlcioplugin.cpp
SOURCES += dmx4linux.cpp

metainfo.path   = $$METAINFODIR
metainfo.files += org.qlcplus.QLCPlus.dmx4linux.metainfo.xml
INSTALLS       += metainfo
