include(../variables.pri)

# Default rules for deployment.
#include(../deployment.pri)

TEMPLATE = app
TARGET = qlcplus-qml

QT += qml quick widgets svg xml script
QT += multimedia multimediawidgets

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Engine
INCLUDEPATH     += ../engine/src ../engine/src/audio
DEPENDPATH      += ../engine/src
QMAKE_LIBDIR    += ../engine/src
LIBS            += -lqlcplusengine
win32:QMAKE_LFLAGS += -shared

HEADERS += \
    app.h \
    fixturebrowser.h \
    fixturemanager.h \
    functionmanager.h \
    inputoutputmanager.h \ 
    treemodel.h \
    treemodelitem.h

SOURCES += main.cpp \
    app.cpp \
    fixturebrowser.cpp \
    fixturemanager.cpp \
    functionmanager.cpp \
    inputoutputmanager.cpp \
    treemodel.cpp \
    treemodelitem.cpp

RESOURCES += qml.qrc ../resources/icons/svg/svgicons.qrc

OTHER_FILES += 

# Installation
target.path = $$INSTALLROOT/$$BINDIR
INSTALLS   += target