include(../../variables.pri)
include(../../coverage.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = qlcplusui

CONFIG += qt
QT     += core xml gui script
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia multimediawidgets

INCLUDEPATH     += monitor showmanager virtualconsole

# Engine
INCLUDEPATH     += ../../engine/src ../../engine/src/audio
DEPENDPATH      += ../../engine/src
QMAKE_LIBDIR    += ../../engine/src
LIBS            += -lqlcplusengine
win32:QMAKE_LFLAGS += -shared

# Types
INCLUDEPATH += ../../plugins/interfaces

# Web Access
INCLUDEPATH     += ../../webaccess

# Resources
RESOURCES    += qlcui.qrc

lessThan(QT_MAJOR_VERSION, 5) {
  macx {
    CONFIG += link_pkgconfig
    system(pkg-config --exists portaudio-2.0) {
      PKGCONFIG += portaudio-2.0
    }
  }
}

# Headers
HEADERS += aboutbox.h \
           addchannelsgroup.h \
           addfixture.h \
           addresstool.h \
           addrgbpanel.h \
           app.h \
           apputil.h \
           assignhotkey.h \
           audiobar.h \
           audioeditor.h \
           audiotriggerwidget.h \
           channelmodifiereditor.h \
           channelmodifiergraphicsview.h \
           channelsselection.h \
           chasereditor.h \
           clickandgoslider.h \
           clickandgowidget.h \
           collectioneditor.h \
           consolechannel.h \
           createfixturegroup.h \
           ctkrangeslider.h \
           cuestackmodel.h \
           docbrowser.h \
           dmxdumpfactory.h \
           efxeditor.h \
           efxpreviewarea.h \
           fixtureconsole.h \
           fixturegroupeditor.h \
           fixturemanager.h \
           fixtureselection.h \
           fixturetreewidget.h \
           functionmanager.h \
           fixtureremap.h \
           flowlayout.h \
           functionliveeditdialog.h \
           functionselection.h \
           functionstreewidget.h \
           functionwizard.h \
           grandmasterslider.h \
           groupsconsole.h \
           inputchanneleditor.h \
           inputoutputmanager.h \
           inputoutputpatcheditor.h \
           inputprofileeditor.h \
           knobwidget.h \
           palettegenerator.h \
           playbackslider.h \
           positiontool.h \
           remapwidget.h \
           rgbmatrixeditor.h \
           rgbitem.h \
           sceneeditor.h \
           scripteditor.h \
           selectinputchannel.h \
           simpledesk.h \
           simpledeskengine.h \
           speeddial.h \
           speeddialwidget.h \
           universeitemwidget.h

# Monitor headers
HEADERS += monitor/monitor.h \
           monitor/monitorbackgroundselection.h \
           monitor/monitorfixture.h \
           monitor/monitorfixtureitem.h \
           monitor/monitorgraphicsview.h \
           monitor/monitorlayout.h \
           monitor/monitorfixturepropertieseditor.h

# Show Manager headers
HEADERS += showmanager/multitrackview.h \
           showmanager/showeditor.h \
           showmanager/headeritems.h \
           showmanager/trackitem.h \
           showmanager/showitem.h \
           showmanager/sequenceitem.h \
           showmanager/audioitem.h \
           showmanager/rgbmatrixitem.h \
           showmanager/efxitem.h \
           showmanager/timingstool.h \
           showmanager/showmanager.h

# Virtual Console headers
HEADERS += virtualconsole/addvcbuttonmatrix.h \
           virtualconsole/addvcslidermatrix.h \
           virtualconsole/vcaudiotriggers.h \
           virtualconsole/vcaudiotriggersproperties.h \
           virtualconsole/vcbutton.h \
           virtualconsole/vcbuttonproperties.h \
           virtualconsole/vcclock.h \
           virtualconsole/vcclockproperties.h \
           virtualconsole/vccuelist.h \
           virtualconsole/vccuelistproperties.h \
           virtualconsole/vcdockarea.h \
           virtualconsole/vcframe.h \
           virtualconsole/vcframeproperties.h \
           virtualconsole/vclabel.h \
           virtualconsole/vcmatrix.h \
           virtualconsole/vcmatrixcontrol.h \
           virtualconsole/vcmatrixpresetselection.h \
           virtualconsole/vcmatrixproperties.h \
           virtualconsole/vcproperties.h \
           virtualconsole/vcpropertieseditor.h \
           virtualconsole/vcslider.h \
           virtualconsole/vcsliderproperties.h \
           virtualconsole/vcsoloframe.h \
           virtualconsole/vcspeeddial.h \
           virtualconsole/vcspeeddialfunction.h \
           virtualconsole/vcspeeddialproperties.h \
           virtualconsole/vcwidget.h \
           virtualconsole/vcwidgetproperties.h \
           virtualconsole/vcwidgetselection.h \
           virtualconsole/vcxypad.h \
           virtualconsole/vcxypadarea.h \
           virtualconsole/vcxypadfixture.h \
           virtualconsole/vcxypadfixtureeditor.h \
           virtualconsole/vcxypadproperties.h \
           virtualconsole/virtualconsole.h

# Forms
FORMS += aboutbox.ui \
         addchannelsgroup.ui \
         addfixture.ui \
         addresstool.ui \
         addrgbpanel.ui \
         assignhotkey.ui \
         audioeditor.ui \
         chasereditor.ui \
         channelmodifiereditor.ui \
         channelsselection.ui \
         collectioneditor.ui \
         createfixturegroup.ui \
         dmxdumpfactory.ui \
         efxeditor.ui \
         fixturegroupeditor.ui \
         fixtureremap.ui \
         fixtureselection.ui \
         functionselection.ui \
         functionwizard.ui \
         inputchanneleditor.ui \
         inputoutputpatcheditor.ui \
         inputprofileeditor.ui \
         monitor/monitorbackgroundselection.ui \
         monitor/monitorfixturepropertieseditor.ui \
         positiontool.ui \
         rgbmatrixeditor.ui \
         sceneeditor.ui \
         scripteditor.ui \
         selectinputchannel.ui \
         showmanager/showeditor.ui

# Virtual Console Forms
FORMS += virtualconsole/addvcbuttonmatrix.ui \
         virtualconsole/addvcslidermatrix.ui \
         virtualconsole/vcaudiotriggersproperties.ui \
         virtualconsole/vcbuttonproperties.ui \
         virtualconsole/vcclockproperties.ui \
         virtualconsole/vccuelistproperties.ui \
         virtualconsole/vcframeproperties.ui \
         virtualconsole/vcmatrixpresetselection.ui \
         virtualconsole/vcmatrixproperties.ui \
         virtualconsole/vcproperties.ui \
         virtualconsole/vcsliderproperties.ui \
         virtualconsole/vcspeeddialproperties.ui \
         virtualconsole/vcwidgetselection.ui \
         virtualconsole/vcxypadfixtureeditor.ui \
         virtualconsole/vcxypadproperties.ui

# Sources
SOURCES += aboutbox.cpp \
           addchannelsgroup.cpp \
           addfixture.cpp \
           addresstool.cpp \
           addrgbpanel.cpp \
           app.cpp \
           apputil.cpp \
           assignhotkey.cpp \
           audiobar.cpp \
           audioeditor.cpp \
           audiotriggerwidget.cpp \
           channelmodifiereditor.cpp \
           channelmodifiergraphicsview.cpp \
           channelsselection.cpp \
           chasereditor.cpp \
           clickandgoslider.cpp \
           clickandgowidget.cpp \
           collectioneditor.cpp \
           consolechannel.cpp \
           createfixturegroup.cpp \
           ctkrangeslider.cpp \
           cuestackmodel.cpp \
           docbrowser.cpp \
           dmxdumpfactory.cpp \
           efxeditor.cpp \
           efxpreviewarea.cpp \
           fixtureconsole.cpp \
           fixturegroupeditor.cpp \
           fixturemanager.cpp \
           fixtureremap.cpp \
           fixtureselection.cpp \
           fixturetreewidget.cpp \
           flowlayout.cpp \
           functionliveeditdialog.cpp \
           functionmanager.cpp \
           functionselection.cpp \
           functionstreewidget.cpp \
           functionwizard.cpp \
           grandmasterslider.cpp \
           groupsconsole.cpp \
           inputchanneleditor.cpp \
           inputoutputmanager.cpp \
           inputoutputpatcheditor.cpp \
           inputprofileeditor.cpp \
           knobwidget.cpp \
           palettegenerator.cpp \
           playbackslider.cpp \
           positiontool.cpp \
           remapwidget.cpp \
           rgbmatrixeditor.cpp \
           rgbitem.cpp \
           sceneeditor.cpp \
           scripteditor.cpp \
           selectinputchannel.cpp \
           simpledesk.cpp \
           simpledeskengine.cpp \
           speeddial.cpp \
           speeddialwidget.cpp \
           universeitemwidget.cpp

# Monitor sources
SOURCES += monitor/monitor.cpp \
           monitor/monitorbackgroundselection.cpp \
           monitor/monitorfixture.cpp \
           monitor/monitorfixtureitem.cpp \
           monitor/monitorgraphicsview.cpp \
           monitor/monitorlayout.cpp \
           monitor/monitorfixturepropertieseditor.cpp

# Show Manager sources
SOURCES += showmanager/multitrackview.cpp \
           showmanager/showeditor.cpp \
           showmanager/headeritems.cpp \
           showmanager/trackitem.cpp \
           showmanager/showitem.cpp \
           showmanager/sequenceitem.cpp \
           showmanager/audioitem.cpp \
           showmanager/rgbmatrixitem.cpp \
           showmanager/efxitem.cpp \
           showmanager/timingstool.cpp \
           showmanager/showmanager.cpp

# Virtual Console sources
SOURCES += virtualconsole/addvcbuttonmatrix.cpp \
           virtualconsole/addvcslidermatrix.cpp \
           virtualconsole/vcaudiotriggers.cpp \
           virtualconsole/vcaudiotriggersproperties.cpp \
           virtualconsole/vcbutton.cpp \
           virtualconsole/vcbuttonproperties.cpp \
           virtualconsole/vcclock.cpp \
           virtualconsole/vcclockproperties.cpp \
           virtualconsole/vccuelist.cpp \
           virtualconsole/vccuelistproperties.cpp \
           virtualconsole/vcdockarea.cpp \
           virtualconsole/vcframe.cpp \
           virtualconsole/vcframeproperties.cpp \
           virtualconsole/vclabel.cpp \
           virtualconsole/vcmatrix.cpp \
           virtualconsole/vcmatrixcontrol.cpp \
           virtualconsole/vcmatrixpresetselection.cpp \
           virtualconsole/vcmatrixproperties.cpp \
           virtualconsole/vcproperties.cpp \
           virtualconsole/vcpropertieseditor.cpp \
           virtualconsole/vcslider.cpp \
           virtualconsole/vcsliderproperties.cpp \
           virtualconsole/vcsoloframe.cpp \
           virtualconsole/vcspeeddial.cpp \
           virtualconsole/vcspeeddialfunction.cpp \
           virtualconsole/vcspeeddialproperties.cpp \
           virtualconsole/vcwidget.cpp \
           virtualconsole/vcwidgetproperties.cpp \
           virtualconsole/vcwidgetselection.cpp \
           virtualconsole/vcxypad.cpp \
           virtualconsole/vcxypadarea.cpp \
           virtualconsole/vcxypadfixture.cpp \
           virtualconsole/vcxypadfixtureeditor.cpp \
           virtualconsole/vcxypadproperties.cpp \
           virtualconsole/virtualconsole.cpp

greaterThan(QT_MAJOR_VERSION, 4) {
HEADERS += videoeditor.h showmanager/videoitem.h videoprovider.h
FORMS += videoeditor.ui
SOURCES += videoeditor.cpp showmanager/videoitem.cpp videoprovider.cpp
}

TRANSLATIONS += qlcplus_fi_FI.ts
TRANSLATIONS += qlcplus_fr_FR.ts
TRANSLATIONS += qlcplus_es_ES.ts
TRANSLATIONS += qlcplus_de_DE.ts
TRANSLATIONS += qlcplus_it_IT.ts
TRANSLATIONS += qlcplus_nl_NL.ts
TRANSLATIONS += qlcplus_cz_CZ.ts
TRANSLATIONS += qlcplus_pt_BR.ts
TRANSLATIONS += qlcplus_ca_ES.ts
TRANSLATIONS += qlcplus_ja_JP.ts
TRANSLATIONS += qlcplus_zh_CN.ts

macx {
    # This must be after "TARGET = " and before target installation so that
    # install_name_tool can be run before target installation
    include(../../macx/nametool.pri)
}

# Installation
target.path = $$INSTALLROOT/$$LIBSDIR
INSTALLS   += target
