include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = artnet

CONFIG      += plugin
CONFIG      += link_pkgconfig
INCLUDEPATH += ../interfaces
DEPENDPATH  += ../interfaces

PKGCONFIG += libartnet
win32:QMAKE_LFLAGS += -shared

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
macx:include(../../macx/nametool.pri)

target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target

TRANSLATIONS += ArtNet_de_DE.ts
TRANSLATIONS += ArtNet_es_ES.ts
TRANSLATIONS += ArtNet_fi_FI.ts
TRANSLATIONS += ArtNet_fr_FR.ts
TRANSLATIONS += ArtNet_it_IT.ts

HEADERS += ../interfaces/qlcioplugin.h
HEADERS += artnetplugin.h

#FORMS += configureartnet.ui

SOURCES += artnetplugin.cpp