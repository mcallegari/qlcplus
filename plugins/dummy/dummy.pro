include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = dummy

QT += widgets

INCLUDEPATH += ../interfaces
CONFIG      += plugin

target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target

TRANSLATIONS += Dummy_de_DE.ts
TRANSLATIONS += Dummy_es_ES.ts
TRANSLATIONS += Dummy_fi_FI.ts
TRANSLATIONS += Dummy_fr_FR.ts
TRANSLATIONS += Dummy_it_IT.ts
TRANSLATIONS += Dummy_nl_NL.ts
TRANSLATIONS += Dummy_cz_CZ.ts
TRANSLATIONS += Dummy_pt_BR.ts
TRANSLATIONS += Dummy_ca_ES.ts
TRANSLATIONS += Dummy_ja_JP.ts

HEADERS += ../interfaces/qlcioplugin.h
HEADERS += dummyplugin.h \
           dummyconfiguration.h

SOURCES += ../interfaces/qlcioplugin.cpp
SOURCES += dummyplugin.cpp \
           dummyconfiguration.cpp

FORMS += dummyconfiguration.ui
