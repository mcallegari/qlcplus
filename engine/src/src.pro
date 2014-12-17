include(../../variables.pri)
include(../../coverage.pri)
include(../../hotplugmonitor/hotplugmonitor.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = qlcplusengine

CONFIG  += qt
QT      += core xml script gui
greaterThan(QT_MAJOR_VERSION, 4) {
  QT += multimedia
  macx:QT_CONFIG -= no-pkg-config
  win32:QT += widgets
}

CONFIG += link_pkgconfig

QTPLUGIN =

INCLUDEPATH += ./audio ../../plugins/interfaces
win32:LIBS  += -lwinmm
win32:QMAKE_LFLAGS += -shared
win32:INCLUDEPATH += ./

DEPENDPATH  += ../../hotplugmonitor/src
INCLUDEPATH += ../../hotplugmonitor/src
LIBS        += -L../../hotplugmonitor/src -lhotplugmonitor



#############################################################################
# Sources
#############################################################################

# Fixture metadata
HEADERS += avolitesd4parser.h \
           qlccapability.h \
           qlcchannel.h \
           qlcfile.h \
           qlcfixturedef.h \
           qlcfixturedefcache.h \
           qlcfixturehead.h \
           qlcfixturemode.h \
           qlci18n.h \
           qlcinputchannel.h \
           qlcinputprofile.h \
           qlcinputsource.h \
           qlcmodifierscache.h \
           qlcphysical.h

# Audio
HEADERS += audio/audio.h \
           audio/audiodecoder.h \
           audio/audiorenderer.h \
           audio/audioparameters.h \
           audio/audiocapture.h

lessThan(QT_MAJOR_VERSION, 5) {
  unix:!macx:HEADERS += audio/audiorenderer_alsa.h audio/audiocapture_alsa.h
  win32:HEADERS += audio/audiorenderer_waveout.h audio/audiocapture_wavein.h
}
else {
  HEADERS += audio/audiorenderer_qt.h audio/audiocapture_qt.h
  HEADERS += video.h
}

# Engine
HEADERS += bus.h \
           channelsgroup.h \
           channelmodifier.h \
           chaser.h \
           chaserrunner.h \
           chaserstep.h \
           collection.h \
           cue.h \
           cuestack.h \
           doc.h \
           dmxdumpfactoryproperties.h \
           dmxsource.h \
           efx.h \
           efxfixture.h \
           efxuistate.h \
           fadechannel.h \
           fixture.h \
           fixturegroup.h \
           function.h \
           functionuistate.h \
           genericdmxsource.h \
           genericfader.h \
           grandmaster.h \
           grouphead.h \
           inputoutputmap.h \
           inputpatch.h \
           ioplugincache.h \
           mastertimer.h \
           monitorproperties.h \
           outputpatch.h \
           qlcclipboard.h \
           qlcpoint.h \
           rgbalgorithm.h \
           rgbaudio.h \
           rgbmatrix.h \
           rgbimage.h \
           rgbplain.h \
           rgbscript.h \
           rgbscriptproperty.h \
           rgbscriptscache.h \
           rgbtext.h \
           scene.h \
           sceneuistate.h \
           scenevalue.h \
           script.h \
           show.h \
           showfunction.h \
           showrunner.h \
           track.h \
           universe.h

win32:HEADERS += mastertimer-win32.h
unix:HEADERS  += mastertimer-unix.h

# Fixture metadata
SOURCES += avolitesd4parser.cpp \
           qlccapability.cpp \
           qlcchannel.cpp \
           qlcfile.cpp \
           qlcfixturedef.cpp \
           qlcfixturedefcache.cpp \
           qlcfixturehead.cpp \
           qlcfixturemode.cpp \
           qlci18n.cpp \
           qlcinputchannel.cpp \
           qlcinputprofile.cpp \
           qlcinputsource.cpp \
           qlcmodifierscache.cpp \
           qlcphysical.cpp

# Audio
SOURCES += audio/audio.cpp \
           audio/audiodecoder.cpp \
           audio/audiorenderer.cpp \
           audio/audioparameters.cpp \
           audio/audiocapture.cpp

lessThan(QT_MAJOR_VERSION, 5) {
  unix:!macx:SOURCES += audio/audiorenderer_alsa.cpp audio/audiocapture_alsa.cpp
  win32:SOURCES += audio/audiorenderer_waveout.cpp audio/audiocapture_wavein.cpp

  macx {
    system(pkg-config --exists portaudio-2.0) {
      DEFINES += HAS_PORTAUDIO
      PKGCONFIG += portaudio-2.0
      HEADERS += audio/audiorenderer_portaudio.h audio/audiocapture_portaudio.h
      SOURCES += audio/audiorenderer_portaudio.cpp audio/audiocapture_portaudio.cpp
    }

  #  HEADERS += audio/audiorenderer_coreaudio.h
  #  SOURCES += audio/audiorenderer_coreaudio.cpp
  }
}
else {
  SOURCES += audio/audiorenderer_qt.cpp audio/audiocapture_qt.cpp
  SOURCES += video.cpp
}

# Engine
SOURCES += bus.cpp \
           channelsgroup.cpp \
           channelmodifier.cpp \
           chaser.cpp \
           chaserrunner.cpp \
           chaserstep.cpp \
           collection.cpp \
           cue.cpp \
           cuestack.cpp \
           doc.cpp \
           dmxdumpfactoryproperties.cpp \
           efx.cpp \
           efxfixture.cpp \
           efxuistate.cpp \
           fadechannel.cpp \
           fixture.cpp \
           fixturegroup.cpp \
           function.cpp \
           functionuistate.cpp \
           genericdmxsource.cpp \
           genericfader.cpp \
           grandmaster.cpp \
           grouphead.cpp \
           inputoutputmap.cpp \
           inputpatch.cpp \
           ioplugincache.cpp \
           mastertimer.cpp \
           monitorproperties.cpp \
           outputpatch.cpp \
           qlcclipboard.cpp \
           qlcpoint.cpp \
           rgbalgorithm.cpp \
           rgbaudio.cpp \
           rgbmatrix.cpp \
           rgbimage.cpp \
           rgbplain.cpp \
           rgbscript.cpp \
           rgbscriptscache.cpp \
           rgbtext.cpp \
           scene.cpp \
           sceneuistate.cpp \
           scenevalue.cpp \
           script.cpp \
           show.cpp \
           showfunction.cpp \
           showrunner.cpp \
           track.cpp \
           universe.cpp

win32:SOURCES += mastertimer-win32.cpp
unix:SOURCES  += mastertimer-unix.cpp

system(pkg-config --exists mad) {
    DEFINES += HAS_LIBMAD
    PKGCONFIG += mad
    HEADERS += audio/audiodecoder_mad.h
    SOURCES += audio/audiodecoder_mad.cpp
}

system(pkg-config --exists sndfile) {
    DEFINES += HAS_LIBSNDFILE
    PKGCONFIG += sndfile
    HEADERS += audio/audiodecoder_sndfile.h
    SOURCES += audio/audiodecoder_sndfile.cpp
}

system(pkg-config --exists fftw3) {
    PKGCONFIG += fftw3
macx:LIBS += -lfftw3
}

unix:!macx:LIBS += -lasound

# Interfaces
HEADERS += ../../plugins/interfaces/qlcioplugin.h

#############################################################################
# Installation
#############################################################################

macx {
    LIBS += -framework CoreFoundation -framework CoreAudio -framework AudioToolbox
    # This must be after "TARGET = " and before target installation so that
    # install_name_tool can be run before target installation
    include(../../macx/nametool.pri)
}

target.path = $$INSTALLROOT/$$LIBSDIR
INSTALLS   += target

#############################################################################
# qlcconfig.h generation
#############################################################################

CONFIGFILE = qlcconfig.h
conf.target = $$CONFIGFILE
QMAKE_EXTRA_TARGETS += conf
PRE_TARGETDEPS += $$CONFIGFILE
QMAKE_CLEAN += $$CONFIGFILE
QMAKE_DISTCLEAN += $$CONFIGFILE

macx {
    conf.commands += echo \"$$LITERAL_HASH ifndef CONFIG_H\" > $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define CONFIG_H\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define APPNAME \\\"$$APPNAME\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define FXEDNAME \\\"$$FXEDNAME\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define APPVERSION \\\"$$APPVERSION\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define DOCSDIR \\\"$$DOCSDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define INPUTPROFILEDIR \\\"$$INPUTPROFILEDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define USERQLCPLUSDIR \\\"$$USERDATADIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define USERINPUTPROFILEDIR \\\"$$USERINPUTPROFILEDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define MIDITEMPLATEDIR \\\"$$MIDITEMPLATEDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define USERMIDITEMPLATEDIR \\\"$$USERMIDITEMPLATEDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define MODIFIERSTEMPLATEDIR \\\"$$MODIFIERSTEMPLATEDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define USERMODIFIERSTEMPLATEDIR \\\"$$USERMODIFIERSTEMPLATEDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define FIXTUREDIR \\\"$$FIXTUREDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define USERFIXTUREDIR \\\"$$USERFIXTUREDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define PLUGINDIR \\\"$$PLUGINDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define TRANSLATIONDIR \\\"$$TRANSLATIONDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define RGBSCRIPTDIR \\\"$$RGBSCRIPTDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define USERRGBSCRIPTDIR \\\"$$USERRGBSCRIPTDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define GOBODIR \\\"$$GOBODIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define WEBFILESDIR \\\"$$WEBFILESDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH endif\" >> $$CONFIGFILE
}
unix:!macx {
    conf.commands += echo \"$$LITERAL_HASH ifndef CONFIG_H\" > $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define CONFIG_H\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define APPNAME \\\"$$APPNAME\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define FXEDNAME \\\"$$FXEDNAME\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define APPVERSION \\\"$$APPVERSION\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define DOCSDIR \\\"$$INSTALLROOT/$$DOCSDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define INPUTPROFILEDIR \\\"$$INSTALLROOT/$$INPUTPROFILEDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define USERQLCPLUSDIR \\\"$$USERDATADIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define USERINPUTPROFILEDIR \\\"$$USERINPUTPROFILEDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define MIDITEMPLATEDIR \\\"$$INSTALLROOT/$$MIDITEMPLATEDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define USERMIDITEMPLATEDIR \\\"$$USERMIDITEMPLATEDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define MODIFIERSTEMPLATEDIR \\\"$$INSTALLROOT/$$MODIFIERSTEMPLATEDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define USERMODIFIERSTEMPLATEDIR \\\"$$USERMODIFIERSTEMPLATEDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define FIXTUREDIR \\\"$$INSTALLROOT/$$FIXTUREDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define USERFIXTUREDIR \\\"$$USERFIXTUREDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define PLUGINDIR \\\"$$INSTALLROOT/$$PLUGINDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define TRANSLATIONDIR \\\"$$INSTALLROOT/$$TRANSLATIONDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define RGBSCRIPTDIR \\\"$$INSTALLROOT/$$RGBSCRIPTDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define USERRGBSCRIPTDIR \\\"$$USERRGBSCRIPTDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define GOBODIR \\\"$$INSTALLROOT/$$GOBODIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define WEBFILESDIR \\\"$$INSTALLROOT/$$WEBFILESDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH endif\" >> $$CONFIGFILE
}
win32 {
    conf.commands += @echo $$LITERAL_HASH ifndef CONFIG_H > $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define CONFIG_H >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define APPNAME \"$$APPNAME\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define FXEDNAME \"$$FXEDNAME\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define APPVERSION \"$$APPVERSION\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define DOCSDIR \"$$DOCSDIR\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define INPUTPROFILEDIR \"$$INPUTPROFILEDIR\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define USERQLCPLUSDIR \"$$USERDATADIR\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define USERINPUTPROFILEDIR \"$$USERINPUTPROFILEDIR\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define MIDITEMPLATEDIR \"$$MIDITEMPLATEDIR\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define USERMIDITEMPLATEDIR \"$$USERMIDITEMPLATEDIR\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define MODIFIERSTEMPLATEDIR \"$$MODIFIERSTEMPLATEDIR\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define USERMODIFIERSTEMPLATEDIR \"$$USERMODIFIERSTEMPLATEDIR\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define FIXTUREDIR \"$$FIXTUREDIR\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define USERFIXTUREDIR \"$$USERFIXTUREDIR\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define PLUGINDIR \"$$PLUGINDIR\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define TRANSLATIONDIR \"$$TRANSLATIONDIR\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define RGBSCRIPTDIR \"$$RGBSCRIPTDIR\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define USERRGBSCRIPTDIR \"$$USERRGBSCRIPTDIR\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define GOBODIR \"$$GOBODIR\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define WEBFILESDIR \"$$WEBFILESDIR\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH endif >> $$CONFIGFILE
}
