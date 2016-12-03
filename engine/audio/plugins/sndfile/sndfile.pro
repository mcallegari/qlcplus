include(../../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = sndfileplugin

INCLUDEPATH += ../../src
CONFIG      += plugin
QT          += core

greaterThan(QT_MAJOR_VERSION, 4) {
    macx:QT_CONFIG -= no-pkg-config
}
CONFIG      += link_pkgconfig
PKGCONFIG   += sndfile

macx {
    # This must be after "TARGET = " and before target installation so that
    # install_name_tool can be run before target installation
    include(../../../../platforms/macos/nametool.pri)
}

target.path = $$INSTALLROOT/$$AUDIOPLUGINDIR
INSTALLS   += target

HEADERS += ../../src/audiodecoder.h ../../src/audioparameters.h
SOURCES += ../../src/audiodecoder.cpp ../../src/audioparameters.cpp
HEADERS += audiodecoder_sndfile.h
SOURCES += audiodecoder_sndfile.cpp
