include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = olaio

QT       += core gui
CONFIG   += plugin
QTPLUGIN  =

INCLUDEPATH += ../interfaces

macx: {
    #CONFIG    += link_pkgconfig
    #PKGCONFIG += libola libolaserver
    INCLUDEPATH += /opt/local/include
    LIBS      += -L/opt/local/lib -lolaserver -lola -lolacommon -lprotobuf
} else {
    LIBS      += -L/usr/local/lib -lolaserver -lola -lolacommon -lprotobuf
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

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
macx {
    include(../../macx/nametool.pri)
}

# Installation
target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target
