include(../../variables.pri)
include(../../coverage.pri)
include(../../hotplugmonitor/hotplugmonitor.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = qlcplusengine

CONFIG  += qt
QT      += core xml script gui
# Uncomment to enable Phonon audio support
#QT += phonon
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
           qlcphysical.h \
           qlccapability.h
# Audio
HEADERS += audio/audio.h \
           audio/audiodecoder.h \
           audio/audiorenderer.h \
           audio/audioparameters.h

unix:!macx:HEADERS += audio/audiorenderer_alsa.h
win32:HEADERS += audio/audiorenderer_waveout.h

# Engine
HEADERS += bus.h \
           channelsgroup.h \
           chaser.h \
           chaserrunner.h \
           chaserstep.h \
           collection.h \
           cue.h \
           cuestack.h \
           doc.h \
           efx.h \
           efxfixture.h \
           fadechannel.h \
           fixture.h \
           fixturegroup.h \
           function.h \
           genericdmxsource.h \
           genericfader.h \
           grouphead.h \
           inputmap.h \
           inputpatch.h \
           intensitygenerator.h \
           ioplugincache.h \
           mastertimer.h \
           universearray.h \
           outputmap.h \
           outputpatch.h \
           palettegenerator.h \
           qlcpoint.h \
           rgbalgorithm.h \
           rgbmatrix.h \
           rgbscript.h \
           rgbtext.h \
           scene.h \
           scenevalue.h \
           script.h \
           show.h \
           showrunner.h \
           track.h

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
           qlcphysical.cpp
# Audio
SOURCES += audio/audio.cpp \
           audio/audiodecoder.cpp \
           audio/audiorenderer.cpp \
           audio/audioparameters.cpp

unix:!macx:SOURCES += audio/audiorenderer_alsa.cpp
win32:SOURCES += audio/audiorenderer_waveout.cpp

macx {
     HEADERS += audio/audiorenderer_coreaudio.h
     SOURCES += audio/audiorenderer_coreaudio.cpp
}

# Engine
SOURCES += bus.cpp \
           channelsgroup.cpp \
           chaser.cpp \
           chaserrunner.cpp \
           chaserstep.cpp \
           collection.cpp \
           cue.cpp \
           cuestack.cpp \
           doc.cpp \
           efx.cpp \
           efxfixture.cpp \
           fadechannel.cpp \
           fixture.cpp \
           fixturegroup.cpp \
           function.cpp \
           genericdmxsource.cpp \
           genericfader.cpp \
           grouphead.cpp \
           inputmap.cpp \
           inputpatch.cpp \
           intensitygenerator.cpp \
           ioplugincache.cpp \
           mastertimer.cpp \
           universearray.cpp \
           outputmap.cpp \
           outputpatch.cpp \
           palettegenerator.cpp \
           qlcpoint.cpp \
           rgbalgorithm.cpp \
           rgbmatrix.cpp \
           rgbscript.cpp \
           rgbtext.cpp \
           scene.cpp \
           scenevalue.cpp \
           script.cpp \
           show.cpp \
           showrunner.cpp \
           track.cpp

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
    conf.commands += echo \"$$LITERAL_HASH define USERINPUTPROFILEDIR \\\"$$USERINPUTPROFILEDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define FIXTUREDIR \\\"$$FIXTUREDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define USERFIXTUREDIR \\\"$$USERFIXTUREDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define PLUGINDIR \\\"$$PLUGINDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define TRANSLATIONDIR \\\"$$TRANSLATIONDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define RGBSCRIPTDIR \\\"$$RGBSCRIPTDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define USERRGBSCRIPTDIR \\\"$$USERRGBSCRIPTDIR\\\"\" >> $$CONFIGFILE &&
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
    conf.commands += echo \"$$LITERAL_HASH define USERINPUTPROFILEDIR \\\"$$USERINPUTPROFILEDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define FIXTUREDIR \\\"$$INSTALLROOT/$$FIXTUREDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define USERFIXTUREDIR \\\"$$USERFIXTUREDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define PLUGINDIR \\\"$$INSTALLROOT/$$PLUGINDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define TRANSLATIONDIR \\\"$$INSTALLROOT/$$TRANSLATIONDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define RGBSCRIPTDIR \\\"$$INSTALLROOT/$$RGBSCRIPTDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define USERRGBSCRIPTDIR \\\"$$USERRGBSCRIPTDIR\\\"\" >> $$CONFIGFILE &&
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
    conf.commands += @echo $$LITERAL_HASH define USERINPUTPROFILEDIR \"$$USERINPUTPROFILEDIR\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define FIXTUREDIR \"$$FIXTUREDIR\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define USERFIXTUREDIR \"$$USERFIXTUREDIR\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define PLUGINDIR \"$$PLUGINDIR\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define TRANSLATIONDIR \"$$TRANSLATIONDIR\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define RGBSCRIPTDIR \"$$RGBSCRIPTDIR\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define USERRGBSCRIPTDIR \"$$USERRGBSCRIPTDIR\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH endif >> $$CONFIGFILE
}
