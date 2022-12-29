include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = e131

QT      += network widgets

CONFIG      += plugin
INCLUDEPATH += ../interfaces
DEPENDPATH  += ../interfaces

win32:QMAKE_LFLAGS += -shared

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
macx:include(../../platforms/macos/nametool.pri)

target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target

TRANSLATIONS += E131_de_DE.ts
TRANSLATIONS += E131_es_ES.ts
TRANSLATIONS += E131_fi_FI.ts
TRANSLATIONS += E131_fr_FR.ts
TRANSLATIONS += E131_it_IT.ts
TRANSLATIONS += E131_nl_NL.ts
TRANSLATIONS += E131_cz_CZ.ts
TRANSLATIONS += E131_pt_BR.ts
TRANSLATIONS += E131_ca_ES.ts
TRANSLATIONS += E131_ja_JP.ts

HEADERS += ../interfaces/qlcioplugin.h
HEADERS += e131packetizer.h \
           e131controller.h \
           e131plugin.h \
           configuree131.h

FORMS += configuree131.ui

SOURCES += ../interfaces/qlcioplugin.cpp
SOURCES += e131packetizer.cpp \
           e131controller.cpp \
           e131plugin.cpp \
           configuree131.cpp

unix:!macx {
   metainfo.path   = $$METAINFODIR
   metainfo.files += org.qlcplus.QLCPlus.e131.metainfo.xml
   INSTALLS       += metainfo
}
