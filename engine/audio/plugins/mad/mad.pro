include(../../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = madplugin

INCLUDEPATH += ../../src
CONFIG      += plugin
QT          += core

macx:QT_CONFIG -= no-pkg-config
CONFIG      += link_pkgconfig
PKGCONFIG   += mad

macx {
    # This must be after "TARGET = " and before target installation so that
    # install_name_tool can be run before target installation
    include(../../../../platforms/macos/nametool.pri)
    nametool.commands += $$pkgConfigNametool(mad, libmad.0.dylib)
}

target.path = $$INSTALLROOT/$$AUDIOPLUGINDIR
INSTALLS   += target

HEADERS += ../../src/audiodecoder.h ../../src/audioparameters.h
SOURCES += ../../src/audiodecoder.cpp ../../src/audioparameters.cpp
HEADERS += audiodecoder_mad.h
SOURCES += audiodecoder_mad.cpp
