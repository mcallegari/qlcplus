include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = osc

QT      += network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG      += plugin
INCLUDEPATH += ../interfaces
DEPENDPATH  += ../interfaces

win32:QMAKE_LFLAGS += -shared

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
macx:include(../../platforms/macos/nametool.pri)

target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target

TRANSLATIONS += OSC_de_DE.ts
TRANSLATIONS += OSC_es_ES.ts
TRANSLATIONS += OSC_fi_FI.ts
TRANSLATIONS += OSC_fr_FR.ts
TRANSLATIONS += OSC_it_IT.ts
TRANSLATIONS += OSC_nl_NL.ts
TRANSLATIONS += OSC_cz_CZ.ts
TRANSLATIONS += OSC_pt_BR.ts
TRANSLATIONS += OSC_ca_ES.ts
TRANSLATIONS += OSC_ja_JP.ts

HEADERS += ../interfaces/qlcioplugin.h
HEADERS += oscpacketizer.h \
           osccontroller.h \
           oscplugin.h \
           configureosc.h

FORMS += configureosc.ui

SOURCES += ../interfaces/qlcioplugin.cpp
SOURCES += oscpacketizer.cpp \
           osccontroller.cpp \
           oscplugin.cpp \
           configureosc.cpp

unix:!macx {
   metainfo.path   = $$INSTALLROOT/share/appdata/
   metainfo.files += qlcplus-osc.metainfo.xml
   INSTALLS       += metainfo 
}
