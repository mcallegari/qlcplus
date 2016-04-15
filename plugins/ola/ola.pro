include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = olaio

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG   += plugin
QTPLUGIN  =

INCLUDEPATH += ../interfaces

macx: {
    #CONFIG    += link_pkgconfig
    #PKGCONFIG += libola libolaserver
    #QMAKE_CXXFLAGS_X86_64 -= -mmacosx-version-min=10.5
    #QMAKE_CXXFLAGS_X86_64 = -mmacosx-version-min=10.7
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
    INCLUDEPATH += /opt/local/include
    LIBS      += -L/opt/local/lib -lolaserver -lola -lolacommon
} else {
    LIBS      += -L/usr/local/lib -lolaserver -lola -lolacommon
}

unix:!macx {
   metainfo.path   = $$INSTALLROOT/share/appdata/
   metainfo.files += qlcplus-ola.metainfo.xml
   INSTALLS       += metainfo 
}


# Forms
FORMS += configureolaio.ui

# Headers
HEADERS += ../interfaces/qlcioplugin.h
HEADERS += olaio.h \
           olaoutthread.h \
           configureolaio.h \
           qlclogdestination.h

# Source
SOURCES += ../interfaces/qlcioplugin.cpp
SOURCES += olaio.cpp \
           olaoutthread.cpp \
           configureolaio.cpp \
           qlclogdestination.cpp

TRANSLATIONS += OLA_fi_FI.ts
TRANSLATIONS += OLA_de_DE.ts
TRANSLATIONS += OLA_es_ES.ts
TRANSLATIONS += OLA_fr_FR.ts
TRANSLATIONS += OLA_it_IT.ts
TRANSLATIONS += OLA_nl_NL.ts
TRANSLATIONS += OLA_cz_CZ.ts
TRANSLATIONS += OLA_pt_BR.ts
TRANSLATIONS += OLA_ca_ES.ts
TRANSLATIONS += OLA_ja_JP.ts

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
macx {
    include(../../macx/nametool.pri)
}

# Installation
target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target
