include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = artnet

QT      += network

CONFIG      += plugin
INCLUDEPATH += ../interfaces
DEPENDPATH  += ../interfaces

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
HEADERS += artnetpacketizer.h \
           artnetcontroller.h \
           artnetplugin.h \
           configureartnet.h

FORMS += configureartnet.ui

SOURCES += artnetpacketizer.cpp \
           artnetcontroller.cpp \
           artnetplugin.cpp \
           configureartnet.cpp
