include(../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = qlcplusaudio
CONFIG  += staticlib

QT      += core
greaterThan(QT_MAJOR_VERSION, 4) {
  QT += multimedia
  macx:QT_CONFIG -= no-pkg-config
  win32:QT += widgets
}

CONFIG += link_pkgconfig

INCLUDEPATH += ../../src ../../../plugins/interfaces

HEADERS += audio.h \
           audiodecoder.h \
           audiorenderer.h \
           audioparameters.h \
           audiocapture.h \
           audioplugincache.h


HEADERS += audiorenderer_qt.h audiocapture_qt.h


SOURCES += audio.cpp \
           audiodecoder.cpp \
           audiorenderer.cpp \
           audioparameters.cpp \
           audiocapture.cpp \
           audioplugincache.cpp

lessThan(QT_MAJOR_VERSION, 5) {
    system(pkg-config --exists portaudio-2.0) {
      DEFINES += HAS_PORTAUDIO
      PKGCONFIG += portaudio-2.0
      HEADERS += audiorenderer_portaudio.h audiocapture_portaudio.h
      SOURCES += audiorenderer_portaudio.cpp audiocapture_portaudio.cpp
    }
}
else {
  SOURCES += audiorenderer_qt.cpp audiocapture_qt.cpp
}

!android:!ios {
  system(pkg-config --exists fftw3) {
    DEFINES += HAS_FFTW3
    PKGCONFIG += fftw3
    macx:LIBS += -lfftw3
  }
}
