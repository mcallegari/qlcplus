include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = gpio

QT += widgets

INCLUDEPATH += ../interfaces
CONFIG      += plugin

target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target

TRANSLATIONS += GPIO_de_DE.ts
TRANSLATIONS += GPIO_es_ES.ts
TRANSLATIONS += GPIO_fi_FI.ts
TRANSLATIONS += GPIO_fr_FR.ts
TRANSLATIONS += GPIO_it_IT.ts
TRANSLATIONS += GPIO_nl_NL.ts
TRANSLATIONS += GPIO_cz_CZ.ts
TRANSLATIONS += GPIO_pt_BR.ts
TRANSLATIONS += GPIO_ca_ES.ts
TRANSLATIONS += GPIO_ja_JP.ts

HEADERS += ../interfaces/qlcioplugin.h
HEADERS += gpioplugin.h \
           gpioreaderthread.h \
           gpioconfiguration.h

SOURCES += ../interfaces/qlcioplugin.cpp
SOURCES += gpioplugin.cpp \
           gpioreaderthread.cpp \
           gpioconfiguration.cpp

FORMS += gpioconfiguration.ui
