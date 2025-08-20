include(../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = qlcplusaudio
CONFIG  += staticlib

QT += core
QT += multimedia
macx:QT_CONFIG -= no-pkg-config
win32:QT += widgets

CONFIG += link_pkgconfig

INCLUDEPATH += ../../src ../../../plugins/interfaces

HEADERS += audio.h \
           audiodecoder.h \
           audiorenderer.h \
           audioparameters.h \
           audiocapture.h \
           audioplugincache.h

lessThan(QT_MAJOR_VERSION, 6) {
  HEADERS += audiorenderer_qt5.h audiocapture_qt5.h
} else {
  HEADERS += audiorenderer_qt6.h audiocapture_qt6.h
}

SOURCES += audio.cpp \
           audiodecoder.cpp \
           audiorenderer.cpp \
           audioparameters.cpp \
           audiocapture.cpp \
           audioplugincache.cpp

lessThan(QT_MAJOR_VERSION, 6) {
  SOURCES += audiorenderer_qt5.cpp audiocapture_qt5.cpp
} else {
  SOURCES += audiorenderer_qt6.cpp audiocapture_qt6.cpp
}

!android:!ios {
  system(pkg-config --exists fftw3) {
    DEFINES += HAS_FFTW3
    PKGCONFIG += fftw3
    macx:LIBS += -lfftw3
  }

  unix:!macx:LIBS += -lasound
}
