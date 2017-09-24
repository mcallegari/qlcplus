include(../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = qlcplus-fixturetool

CONFIG += qt

INCLUDEPATH += ../plugins/interfaces

INCLUDEPATH += ../engine/src
DEPENDPATH  += ../engine/src

QMAKE_LIBDIR += ../engine/src
LIBS    += -lqlcplusengine

# Sources

SOURCES += main.cpp

macx {
    # This must be after "TARGET = " and before target installation so that
    # install_name_tool can be run before target installation
    include(../platforms/macos/nametool.pri)
}

# Installation
target.path = $$INSTALLROOT/$$BINDIR
INSTALLS   += target
