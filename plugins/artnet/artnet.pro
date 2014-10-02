include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = artnet

QT      += network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

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
TRANSLATIONS += ArtNet_nl_NL.ts
TRANSLATIONS += ArtNet_cz_CZ.ts
TRANSLATIONS += ArtNet_pt_BR.ts
TRANSLATIONS += ArtNet_ca_ES.ts
TRANSLATIONS += ArtNet_ja_JP.ts

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
