include(../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = qlcplus-fixtureeditor

CONFIG += qt
QT     += widgets
qmlui|greaterThan(QT_MAJOR_VERSION, 5) {
  QT   += qml
} else {
  QT   += script
}

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
           addchannelsdialog.h \
           app.h \
           capabilitywizard.h \
           editchannel.h \
           edithead.h \
           editmode.h \
           editphysical.h \
           fixtureeditor.h \
           util.h

FORMS += ../ui/src/aboutbox.ui \
         addchannelsdialog.ui \
         capabilitywizard.ui \
         editchannel.ui \
         edithead.ui \
         editmode.ui \
         editphysical.ui \
         fixtureeditor.ui

SOURCES += ../ui/src/aboutbox.cpp \
           ../ui/src/docbrowser.cpp \
           ../ui/src/apputil.cpp \
           addchannelsdialog.cpp \
           app.cpp \
           capabilitywizard.cpp \
           editchannel.cpp \
           edithead.cpp \
           editmode.cpp \
           editphysical.cpp \
           fixtureeditor.cpp \
           main.cpp

TRANSLATIONS += fixtureeditor_fi_FI.ts
TRANSLATIONS += fixtureeditor_fr_FR.ts
TRANSLATIONS += fixtureeditor_de_DE.ts
TRANSLATIONS += fixtureeditor_es_ES.ts
TRANSLATIONS += fixtureeditor_it_IT.ts
TRANSLATIONS += fixtureeditor_nl_NL.ts
TRANSLATIONS += fixtureeditor_cz_CZ.ts
TRANSLATIONS += fixtureeditor_pt_BR.ts
TRANSLATIONS += fixtureeditor_ca_ES.ts
TRANSLATIONS += fixtureeditor_ja_JP.ts

macx {
    # This must be after "TARGET = " and before target installation so that
    # install_name_tool can be run before target installation
    include(../platforms/macos/nametool.pri)
}

# Installation
target.path = $$INSTALLROOT/$$BINDIR
INSTALLS   += target
