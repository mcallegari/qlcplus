include(../variables.pri)

# Default rules for deployment.
#include(../deployment.pri)

TEMPLATE = app
TARGET = qlcplus-qml

QT += qml quick widgets svg script
QT += multimedia multimediawidgets

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Engine
INCLUDEPATH     += ../engine/src ../engine/src/audio
INCLUDEPATH     += virtualconsole
DEPENDPATH      += ../engine/src
QMAKE_LIBDIR    += ../engine/src
LIBS            += -lqlcplusengine
win32:QMAKE_LFLAGS += -shared

# Plugins
INCLUDEPATH     += ../plugins/interfaces

HEADERS += \
    app.h \
    chasereditor.h \
    contextmanager.h \
    fixturebrowser.h \
    fixturemanager.h \
    functioneditor.h \
    functionmanager.h \
    inputoutputmanager.h \ 
    treemodel.h \
    treemodelitem.h \
    previewcontext.h \
    mainview2d.h \
    mainviewdmx.h \
    modelselector.h \
    sceneeditor.h \
    showmanager.h

SOURCES += main.cpp \
    app.cpp \
    chasereditor.cpp \
    contextmanager.cpp \
    fixturebrowser.cpp \
    fixturemanager.cpp \
    functioneditor.cpp \
    functionmanager.cpp \
    inputoutputmanager.cpp \
    treemodel.cpp \
    treemodelitem.cpp \
    previewcontext.cpp \
    mainview2d.cpp \
    mainviewdmx.cpp \
    modelselector.cpp \
    sceneeditor.cpp \
    showmanager.cpp
    

#############################################
#  Virtual Console
#############################################

HEADERS += \
    virtualconsole/virtualconsole.h \
    virtualconsole/vcwidget.h \
    virtualconsole/vcframe.h \
    virtualconsole/vcsoloframe.h \
    virtualconsole/vcbutton.h \
    virtualconsole/vclabel.h

SOURCES += \
    virtualconsole/virtualconsole.cpp \
    virtualconsole/vcwidget.cpp \
    virtualconsole/vcframe.cpp \
    virtualconsole/vcsoloframe.cpp \
    virtualconsole/vcbutton.cpp \
    virtualconsole/vclabel.cpp

RESOURCES += qmlui.qrc ../resources/icons/svg/svgicons.qrc ../resources/fonts/fonts.qrc

macx {
    # This must be after "TARGET = " and before target installation so that
    # install_name_tool can be run before target installation
    include(../macx/nametool.pri)
}

# Installation
target.path = $$INSTALLROOT/$$BINDIR
INSTALLS   += target

android: ANDROID_PACKAGE_SOURCE_DIR = $$PWD/../android-files

ios: {
    ios_icon.files = $$files($$PWD/../ios-files/qlcplus*.png)
    QMAKE_BUNDLE_DATA += ios_icon

    fixtures.files += $$files($$PWD/../resources/fixtures/FixturesMap.xml)
    fixtures.files += $$files($$PWD/../resources/fixtures/*.qxf)
    fixtures.path = Fixtures
    QMAKE_BUNDLE_DATA += fixtures

    QMAKE_INFO_PLIST = $$PWD/../ios-files/Info.plist
}
