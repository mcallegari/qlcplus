include(../../../variables.pri)
include(../../../coverage.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = loopback
CONFIG  += plugin
win32:DEFINES += QLC_EXPORT

INCLUDEPATH += ../../interfaces

HEADERS += loopback.h
SOURCES += loopback.cpp
HEADERS += ../../interfaces/qlcioplugin.h

TRANSLATIONS += loopback_fi_FI.ts
TRANSLATIONS += loopback_de_DE.ts
TRANSLATIONS += loopback_es_ES.ts
TRANSLATIONS += loopback_fr_FR.ts
TRANSLATIONS += loopback_it_IT.ts
TRANSLATIONS += loopback_nl_NL.ts
TRANSLATIONS += loopback_cz_CZ.ts
TRANSLATIONS += loopback_pt_BR.ts
TRANSLATIONS += loopback_ca_ES.ts
TRANSLATIONS += loopback_ja_JP.ts

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
macx:include(../../../macx/nametool.pri)

target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target
