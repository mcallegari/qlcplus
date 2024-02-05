include(../../../variables.pri)
include(../../../coverage.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = enttecwing

INCLUDEPATH   += ../../interfaces
CONFIG        += plugin
QT            += network widgets
win32:DEFINES += QLC_EXPORT
QTPLUGIN       =

win32 {
    # Qt Libraries
    qtnetwork.path = $$INSTALLROOT/$$LIBSDIR
    CONFIG(release, debug|release) qtnetwork.files = $$(QTDIR)/bin/QtNetwork4.dll
    CONFIG(debug, debug|release) qtnetwork.files = $$(QTDIR)/bin/QtNetworkd4.dll
    INSTALLS    += qtnetwork
	QMAKE_LFLAGS += -shared
}

# Input
HEADERS += ../../interfaces/qlcioplugin.h
HEADERS += enttecwing.h \
           playbackwing.h \
           shortcutwing.h \
           programwing.h \
           wing.h

SOURCES += ../../interfaces/qlcioplugin.cpp
SOURCES += enttecwing.cpp \
           playbackwing.cpp \
           shortcutwing.cpp \
           programwing.cpp \
           wing.cpp

TRANSLATIONS += ENTTEC_Wing_fi_FI.ts
TRANSLATIONS += ENTTEC_Wing_de_DE.ts
TRANSLATIONS += ENTTEC_Wing_es_ES.ts
TRANSLATIONS += ENTTEC_Wing_fr_FR.ts
TRANSLATIONS += ENTTEC_Wing_it_IT.ts
TRANSLATIONS += ENTTEC_Wing_nl_NL.ts
TRANSLATIONS += ENTTEC_Wing_cz_CZ.ts
TRANSLATIONS += ENTTEC_Wing_pt_BR.ts
TRANSLATIONS += ENTTEC_Wing_ca_ES.ts
TRANSLATIONS += ENTTEC_Wing_ja_JP.ts

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
macx {
    include(../../../platforms/macos/nametool.pri)
}

# Installation
target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target

unix:!macx {
   metainfo.path   = $$METAINFODIR
   metainfo.files += org.qlcplus.QLCPlus.enttecwing.metainfo.xml
   INSTALLS       += metainfo
}
