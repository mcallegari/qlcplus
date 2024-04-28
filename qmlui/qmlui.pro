include(../variables.pri)

# Default rules for deployment.
#include(../deployment.pri)

TEMPLATE = app
TARGET = qlcplus-qml

QT += qml quick widgets svg
QT += multimedia multimediawidgets
QT += printsupport
QT += 3dcore 3drender 3dinput 3dquick 3dquickextras

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Engine
INCLUDEPATH     += ../engine/src ../engine/audio/src
INCLUDEPATH     += virtualconsole
INCLUDEPATH     += fixtureeditor
INCLUDEPATH     += tardis
INCLUDEPATH     += ../plugins/interfaces
INCLUDEPATH     += ../plugins/midi/src/common
DEPENDPATH      += ../engine/src
QMAKE_LIBDIR    += ../engine/src

android {
    LIBS            += -lqlcplusengine_$${QT_ARCH}
} else {
    LIBS            += -lqlcplusengine
}


#win32:QMAKE_LFLAGS += -shared
win32:RC_FILE = qmlui.rc

HEADERS += \
    app.h \
    audioeditor.h \
    chasereditor.h \
    collectioneditor.h \
    colorfilters.h \
    contextmanager.h \
    efxeditor.h \
    fixturebrowser.h \
    fixturegroupeditor.h \
    fixturemanager.h \
    fixtureutils.h \
    functioneditor.h \
    functionmanager.h \
    importmanager.h \
    inputoutputmanager.h \
    inputprofileeditor.h \
    listmodel.h \
    mainview2d.h \
    mainview3d.h \
    mainviewdmx.h \
    modelselector.h \
    palettemanager.h \
    previewcontext.h \
    rgbmatrixeditor.h \
    sceneeditor.h \
    scripteditor.h \
    showmanager.h \
    simpledesk.h \
    treemodel.h \
    treemodelitem.h \
    uimanager.h \
    videoeditor.h \
    videoprovider.h

SOURCES += main.cpp \
    app.cpp \
    audioeditor.cpp \
    chasereditor.cpp \
    collectioneditor.cpp \
    colorfilters.cpp \
    contextmanager.cpp \
    efxeditor.cpp \
    fixturebrowser.cpp \
    fixturegroupeditor.cpp \
    fixturemanager.cpp \
    fixtureutils.cpp \
    functioneditor.cpp \
    functionmanager.cpp \
    importmanager.cpp \
    inputoutputmanager.cpp \
    inputprofileeditor.cpp \
    listmodel.cpp \
    mainview2d.cpp \
    mainview3d.cpp \
    mainviewdmx.cpp \
    modelselector.cpp \
    palettemanager.cpp \
    previewcontext.cpp \
    rgbmatrixeditor.cpp \
    sceneeditor.cpp \
    scripteditor.cpp \
    showmanager.cpp \
    simpledesk.cpp \
    treemodel.cpp \
    treemodelitem.cpp \
    uimanager.cpp \
    videoeditor.cpp \
    videoprovider.cpp

#############################################
#  TARDIS
#############################################

HEADERS += \
    tardis/tardis.h \
    tardis/networkpacketizer.h \
    tardis/networkmanager.h \
    tardis/simplecrypt.h

SOURCES += \
    tardis/tardis.cpp \
    tardis/networkpacketizer.cpp \
    tardis/networkmanager.cpp \
    tardis/simplecrypt.cpp

#############################################
#  Virtual Console
#############################################

HEADERS += \
    virtualconsole/virtualconsole.h \
    virtualconsole/vcwidget.h \
    virtualconsole/vcframe.h \
    virtualconsole/vcsoloframe.h \
    virtualconsole/vcpage.h \
    virtualconsole/vcbutton.h \
    virtualconsole/vclabel.h \
    virtualconsole/vcslider.h \
    virtualconsole/vcanimation.h \
    virtualconsole/vcaudiotrigger.h \
    virtualconsole/vcxypad.h \
    virtualconsole/vcspeeddial.h \
    virtualconsole/vcclock.h \
    virtualconsole/vccuelist.h

SOURCES += \
    virtualconsole/virtualconsole.cpp \
    virtualconsole/vcwidget.cpp \
    virtualconsole/vcframe.cpp \
    virtualconsole/vcsoloframe.cpp \
    virtualconsole/vcpage.cpp \
    virtualconsole/vcbutton.cpp \
    virtualconsole/vclabel.cpp \
    virtualconsole/vcslider.cpp \
    virtualconsole/vcanimation.cpp \
    virtualconsole/vcaudiotrigger.cpp \
    virtualconsole/vcxypad.cpp \
    virtualconsole/vcspeeddial.cpp \
    virtualconsole/vcclock.cpp \
    virtualconsole/vccuelist.cpp

#############################################
#  Fixture Definition Editor
#############################################

HEADERS += \
    fixtureeditor/fixtureeditor.h \
    fixtureeditor/editorview.h \
    fixtureeditor/channeledit.h \
    fixtureeditor/modeedit.h \
    fixtureeditor/physicaledit.h

SOURCES += \
    fixtureeditor/fixtureeditor.cpp \
    fixtureeditor/editorview.cpp \
    fixtureeditor/channeledit.cpp \
    fixtureeditor/modeedit.cpp \
    fixtureeditor/physicaledit.cpp

RESOURCES += qmlui.qrc ../resources/icons/svg/svgicons.qrc ../resources/fonts/fonts.qrc

lupdate_only {
    SOURCES += \
        qml/*.qml \
        qml/fixturesfunctions/*.qml \
        qml/inputoutput/*.qml \
        qml/popup/*.qml \
        qml/showmanager/*.qml \
        qml/virtualconsole/*.qml
}

TRANSLATIONS += \
    qlcplus_ca_ES.ts \
    qlcplus_de_DE.ts \
    qlcplus_es_ES.ts \
    qlcplus_fr_FR.ts \
    qlcplus_it_IT.ts \
    qlcplus_ja_JP.ts \
    qlcplus_nl_NL.ts \
    qlcplus_pl_PL.ts \
    qlcplus_ru_RU.ts \
    qlcplus_uk_UA.ts

macx {
    # This must be after "TARGET = " and before target installation so that
    # install_name_tool can be run before target installation
    include(../platforms/macos/nametool.pri)
}

# Installation
target.path = $$INSTALLROOT/$$BINDIR
INSTALLS   += target

android: ANDROID_PACKAGE_SOURCE_DIR = $$PWD/../platforms/android

ios: {
    ios_icon.files = $$files($$PWD/../platforms/ios/qlcplus*.png)
    QMAKE_BUNDLE_DATA += ios_icon

    fixtures.files += $$files($$PWD/../resources/fixtures/FixturesMap.xml)
    fixtures.files += $$files($$PWD/../resources/fixtures/*.qxf)
    fixtures.path = Fixtures
    QMAKE_BUNDLE_DATA += fixtures

    QMAKE_INFO_PLIST = $$PWD/../platforms/ios/Info.plist
}
