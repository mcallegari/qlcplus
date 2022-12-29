include(../../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = sndfileplugin

INCLUDEPATH += ../../src
CONFIG      += plugin
QT          += core

macx:QT_CONFIG -= no-pkg-config
CONFIG      += link_pkgconfig
PKGCONFIG   += sndfile

macx {
    # This must be after "TARGET = " and before target installation so that
    # install_name_tool can be run before target installation
    include(../../../../platforms/macos/nametool.pri)
    nametool.commands += $$pkgConfigNametool(sndfile, libsndfile.1.dylib)
}

target.path = $$INSTALLROOT/$$AUDIOPLUGINDIR
INSTALLS   += target

HEADERS += ../../src/audiodecoder.h ../../src/audioparameters.h
SOURCES += ../../src/audiodecoder.cpp ../../src/audioparameters.cpp
HEADERS += audiodecoder_sndfile.h
SOURCES += audiodecoder_sndfile.cpp
