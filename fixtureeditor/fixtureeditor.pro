include(../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = qlcplus-fixtureeditor

CONFIG += qt
QT     += xml script

INCLUDEPATH += ../plugins/interfaces

INCLUDEPATH += ../engine/src
DEPENDPATH  += ../engine/src

INCLUDEPATH += ../ui/src
DEPENDPATH  += ../ui/src
QMAKE_LIBDIR += ../engine/src
LIBS    += -lqlcplusengine

# Sources
RESOURCES    += ../ui/src/qlcui.qrc
win32:RC_FILE = fixtureeditor.rc

HEADERS += ../ui/src/aboutbox.h \
           ../ui/src/docbrowser.h \
           ../ui/src/apputil.h \
           app.h \
           capabilitywizard.h \
           editcapability.h \
           editchannel.h \
           edithead.h \
           editmode.h \
           fixtureeditor.h \
           util.h \
    addchannelsdialog.h

FORMS += ../ui/src/aboutbox.ui \
         capabilitywizard.ui \
         editcapability.ui \
         editchannel.ui \
         edithead.ui \
         editmode.ui \
         fixtureeditor.ui \
    addchannelsdialog.ui

SOURCES += ../ui/src/aboutbox.cpp \
           ../ui/src/docbrowser.cpp \
           ../ui/src/apputil.cpp \
           app.cpp \
           capabilitywizard.cpp \
           editcapability.cpp \
           editchannel.cpp \
           edithead.cpp \
           editmode.cpp \
           fixtureeditor.cpp \
           main.cpp \
    addchannelsdialog.cpp

TRANSLATIONS += fixtureeditor_fi_FI.ts
TRANSLATIONS += fixtureeditor_fr_FR.ts
TRANSLATIONS += fixtureeditor_de_DE.ts
TRANSLATIONS += fixtureeditor_es_ES.ts
TRANSLATIONS += fixtureeditor_it_IT.ts
TRANSLATIONS += fixtureeditor_nl_NL.ts
TRANSLATIONS += fixtureeditor_cz_CZ.ts

macx {
    # This must be after "TARGET = " and before target installation so that
    # install_name_tool can be run before target installation
    include(../macx/nametool.pri)
}

# Installation
target.path = $$INSTALLROOT/$$BINDIR
INSTALLS   += target
