include(../../../variables.pri)
include(../../../coverage.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = velleman
CONFIG  += plugin
win32:DEFINES += QLC_EXPORT

INCLUDEPATH += ../../interfaces

win32: {
    # K8062D is a proprietary interface by Velleman and would therefore taint the
    # 100% FLOSS codebase of QLC if distributed along with QLC sources. Download
    # the package from http://www.box.net/shared/2l0b2tk8e1 and
    # extract its contents under K8062DDIR below to compile this plugin.
    K8062DDIR    = C:/Qt/K8062D
    LIBS        += -L$$K8062DDIR -lK8062D
    LIBS        += $$K8062DDIR/K8062D.a
    INCLUDE     += -I$$K8062DDIR
    QMAKE_LFLAGS += -shared
} else {
    SOURCES += velleman_mock.cpp
}

HEADERS += ../../interfaces/qlcioplugin.h
HEADERS += velleman.h

SOURCES += ../../interfaces/qlcioplugin.cpp
SOURCES += velleman.cpp

TRANSLATIONS += Velleman_fi_FI.ts
TRANSLATIONS += Velleman_de_DE.ts
TRANSLATIONS += Velleman_es_ES.ts
TRANSLATIONS += Velleman_fr_FR.ts
TRANSLATIONS += Velleman_it_IT.ts
TRANSLATIONS += Velleman_nl_NL.ts
TRANSLATIONS += Velleman_cz_CZ.ts
TRANSLATIONS += Velleman_pt_BR.ts
TRANSLATIONS += Velleman_ca_ES.ts
TRANSLATIONS += Velleman_ja_JP.ts

# Installation only on Windows; Unix targets are built only for unit testing.
target.path = $$INSTALLROOT/$$PLUGINDIR
win32:INSTALLS   += target
