include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = olaio

QT       += core gui widgets
CONFIG   += plugin
QTPLUGIN  =

INCLUDEPATH += ../interfaces

macx: {
    #QMAKE_CXXFLAGS_X86_64 -= -mmacosx-version-min=10.5
    #QMAKE_CXXFLAGS_X86_64 = -mmacosx-version-min=10.7
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
    QMAKE_CXXFLAGS_WARN_ON += -Wno-deprecated-declarations

    # Check for pkg-config and setup queries accordingly.
    # Otherwise, use MacPorts default paths.
    packagesExist(libola libolaserver){
        CONFIG    += link_pkgconfig
        PKGCONFIG += libola libolaserver
    } else {
        INCLUDEPATH += /opt/local/include
        LIBS      += -L/opt/local/lib -lolaserver -lola -lolacommon
    }
} else {
    LIBS      += -L/usr/local/lib -lolaserver -lola -lolacommon
}

unix:!macx {
   metainfo.path   = $$METAINFODIR
   metainfo.files += org.qlcplus.QLCPlus.ola.metainfo.xml
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
    include(../../platforms/macos/nametool.pri)
}

# Installation
target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target
