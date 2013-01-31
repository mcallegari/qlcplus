include(../variables.pri)

TEMPLATE = app
TARGET = qlcplus-launcher

QT += core gui

RESOURCES += launcher.qrc

INCLUDEPATH += ../plugins/interfaces
INCLUDEPATH += ../engine/src

HEADERS += launcher.h
SOURCES += launcher.cpp main.cpp

TRANSLATIONS += launcher_fi_FI.ts
TRANSLATIONS += launcher_de_DE.ts
TRANSLATIONS += launcher_fr_FR.ts
TRANSLATIONS += launcher_es_ES.ts
TRANSLATIONS += launcher_it_IT.ts

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
include(../macx/nametool.pri)

# Installation
target.path     = $$INSTALLROOT/$$BINDIR
INSTALLS        += target
