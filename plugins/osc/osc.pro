include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = osc

greaterThan(QT_MAJOR_VERSION, 4) {
  QT += widgets
  macx:QT_CONFIG -= no-pkg-config
}

CONFIG      += plugin
CONFIG      += link_pkgconfig
INCLUDEPATH += ../interfaces
DEPENDPATH  += ../interfaces

PKGCONFIG += liblo
win32:QMAKE_LFLAGS += -shared

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
macx:include(../../macx/nametool.pri)

target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target

TRANSLATIONS += OSC_de_DE.ts
TRANSLATIONS += OSC_es_ES.ts
TRANSLATIONS += OSC_fi_FI.ts
TRANSLATIONS += OSC_fr_FR.ts
TRANSLATIONS += OSC_it_IT.ts
TRANSLATIONS += OSC_nl_NL.ts
TRANSLATIONS += OSC_cz_CZ.ts

HEADERS += ../interfaces/qlcioplugin.h
HEADERS += oscplugin.h \
           configureosc.h

FORMS += configureosc.ui

SOURCES += oscplugin.cpp \
           configureosc.cpp

