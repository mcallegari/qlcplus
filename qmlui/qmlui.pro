include(../variables.pri)

# Default rules for deployment.
include(../deployment.pri)

TEMPLATE = app
TARGET = qlcplus-qml

QT += qml quick widgets svg xml
QT += multimedia multimediawidgets

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Engine
INCLUDEPATH     += ../engine/src ../engine/src/audio
DEPENDPATH      += ../engine/src
QMAKE_LIBDIR    += ../engine/src
LIBS            += -lqlcplusengine
win32:QMAKE_LFLAGS += -shared

SOURCES += main.cpp \
    app.cpp \
    fixturebrowser.cpp \
    inputoutputmanager.cpp \
    fixturemanager.cpp

RESOURCES += qml.qrc

OTHER_FILES += 

HEADERS += \
    app.h \
    fixturebrowser.h \
    inputoutputmanager.h \
    fixturemanager.h

# Installation
target.path = $$INSTALLROOT/$$BINDIR
INSTALLS   += target