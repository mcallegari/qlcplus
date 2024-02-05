include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = os2l

QT      += network widgets

INCLUDEPATH += ../interfaces
CONFIG      += plugin

target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target

TRANSLATIONS += OS2L_de_DE.ts
TRANSLATIONS += OS2L_es_ES.ts
#TRANSLATIONS += OS2L_fi_FI.ts
TRANSLATIONS += OS2L_fr_FR.ts
TRANSLATIONS += OS2L_it_IT.ts
TRANSLATIONS += OS2L_nl_NL.ts
#TRANSLATIONS += OS2L_cz_CZ.ts
#TRANSLATIONS += OS2L_pt_BR.ts
TRANSLATIONS += OS2L_ca_ES.ts
TRANSLATIONS += OS2L_ja_JP.ts

HEADERS += ../interfaces/qlcioplugin.h
HEADERS += os2lplugin.h \
           os2lconfiguration.h

SOURCES += ../interfaces/qlcioplugin.cpp
SOURCES += os2lplugin.cpp \
           os2lconfiguration.cpp

FORMS += os2lconfiguration.ui

unix:!macx {
   metainfo.path   = $$METAINFODIR
   metainfo.files += org.qlcplus.QLCPlus.os2l.metainfo.xml
   INSTALLS       += metainfo
}
