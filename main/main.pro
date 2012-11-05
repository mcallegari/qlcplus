include(../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = qlc

INCLUDEPATH  += ../ui/src
INCLUDEPATH  += ../engine/src

QMAKE_LIBDIR += ../ui/src
QMAKE_LIBDIR += ../engine/src
LIBS         += -lqlcengine
LIBS         += -lqlcui

SOURCES      += main.cpp
win32:RC_FILE = main.rc

QT += xml gui core script

macx {
    # This must be after "TARGET = " and before target installation so that
    # install_name_tool can be run before target installation
    include(../macx/nametool.pri)
}

# Installation
target.path = $$INSTALLROOT/$$BINDIR
INSTALLS   += target
