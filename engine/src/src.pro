include(../../variables.pri)
!android:!ios {
 include(../../coverage.pri)
 include(../../hotplugmonitor/hotplugmonitor.pri)
}

TEMPLATE = lib
LANGUAGE = C++
TARGET   = qlcplusengine

QT += core gui
QT += multimedia
macx:QT_CONFIG -= no-pkg-config
win32:QT += widgets

qmlui|greaterThan(QT_MAJOR_VERSION, 5) {
  QT += qml
} else {
  QT += script
}

CONFIG += link_pkgconfig

#QTPLUGIN =

INCLUDEPATH += ../audio/src ../../plugins/interfaces
win32:LIBS  += -lwinmm
win32:QMAKE_LFLAGS += -shared
win32:INCLUDEPATH += ./

!android:!ios {
DEPENDPATH  += ../../hotplugmonitor/src
INCLUDEPATH += ../../hotplugmonitor/src
LIBS        += -L../../hotplugmonitor/src -lhotplugmonitor
}

LIBS        += -L../audio/src -lqlcplusaudio

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
           qlcinputfeedback.h \
           qlcinputprofile.h \
           qlcinputsource.h \
           qlcmodifierscache.h \
           qlcpalette.h \
           qlcphysical.h \
           utils.h

# Engine
HEADERS += bus.h \
           channelsgroup.h \
           channelmodifier.h \
           chaser.h \
           chaseraction.h \
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
           fadechannel.h \
           fixture.h \
           fixturegroup.h \
           function.h \
           genericdmxsource.h \
           genericfader.h \
           gradient.h \
           grandmaster.h \
           grouphead.h \
           inputoutputmap.h \
           inputpatch.h \
           ioplugincache.h \
           keypadparser.h \
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
           rgbscriptproperty.h \
           rgbscriptscache.h \
           rgbtext.h \
           scene.h \
           scenevalue.h \
           scriptwrapper.h \
           sequence.h \
           show.h \
           showfunction.h \
           showrunner.h \
           track.h \
           universe.h \
           video.h

qmlui|greaterThan(QT_MAJOR_VERSION, 5) {
  HEADERS += rgbscriptv4.h scriptrunner.h scriptv4.h
} else {
  HEADERS += rgbscript.h script.h
}

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
           qlcinputfeedback.cpp \
           qlcinputprofile.cpp \
           qlcinputsource.cpp \
           qlcmodifierscache.cpp \
           qlcpalette.cpp \
           qlcphysical.cpp

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
           fadechannel.cpp \
           fixture.cpp \
           fixturegroup.cpp \
           function.cpp \
           genericdmxsource.cpp \
           genericfader.cpp \
           gradient.cpp \
           grandmaster.cpp \
           grouphead.cpp \
           inputoutputmap.cpp \
           inputpatch.cpp \
           ioplugincache.cpp \
           keypadparser.cpp \
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
           rgbscriptscache.cpp \
           rgbtext.cpp \
           scene.cpp \
           scenevalue.cpp \
           sequence.cpp \
           show.cpp \
           showfunction.cpp \
           showrunner.cpp \
           track.cpp \
           universe.cpp \
           video.cpp

qmlui|greaterThan(QT_MAJOR_VERSION, 5) {
  SOURCES += rgbscriptv4.cpp scriptrunner.cpp scriptv4.cpp
} else {
  SOURCES += rgbscript.cpp script.cpp
}

win32:SOURCES += mastertimer-win32.cpp
unix:SOURCES  += mastertimer-unix.cpp

!android:!ios {
  system(pkg-config --exists fftw3) {
    DEFINES += HAS_FFTW3
    PKGCONFIG += fftw3
    macx:LIBS += -lfftw3
  }

  unix:!macx:LIBS += -lasound
}

# Interfaces
HEADERS += ../../plugins/interfaces/qlcioplugin.h
SOURCES += ../../plugins/interfaces/qlcioplugin.cpp

#############################################################################
# Installation
#############################################################################

macx {
    LIBS += -framework CoreFoundation -framework CoreAudio -framework AudioToolbox
    # This must be after "TARGET = " and before target installation so that
    # install_name_tool can be run before target installation
    include(../../platforms/macos/nametool.pri)
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

macx|win32|appimage {
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
    conf.commands += echo \"$$LITERAL_HASH define AUDIOPLUGINDIR \\\"$$AUDIOPLUGINDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define TRANSLATIONDIR \\\"$$TRANSLATIONDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define RGBSCRIPTDIR \\\"$$RGBSCRIPTDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define USERRGBSCRIPTDIR \\\"$$USERRGBSCRIPTDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define GOBODIR \\\"$$GOBODIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define WEBFILESDIR \\\"$$WEBFILESDIR\\\"\" >> $$CONFIGFILE &&
qmlui {
    conf.commands += echo \"$$LITERAL_HASH define MESHESDIR \\\"$$MESHESDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define COLORFILTERSDIR \\\"$$COLORFILTERSDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define USERCOLORFILTERSDIR \\\"$$USERCOLORFILTERSDIR\\\"\" >> $$CONFIGFILE &&
}
    conf.commands += echo \"$$LITERAL_HASH endif\" >> $$CONFIGFILE
}
else:unix|android|ios {
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
    conf.commands += echo \"$$LITERAL_HASH define AUDIOPLUGINDIR \\\"$$INSTALLROOT/$$AUDIOPLUGINDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define TRANSLATIONDIR \\\"$$INSTALLROOT/$$TRANSLATIONDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define RGBSCRIPTDIR \\\"$$INSTALLROOT/$$RGBSCRIPTDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define USERRGBSCRIPTDIR \\\"$$USERRGBSCRIPTDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define GOBODIR \\\"$$INSTALLROOT/$$GOBODIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define WEBFILESDIR \\\"$$INSTALLROOT/$$WEBFILESDIR\\\"\" >> $$CONFIGFILE &&
qmlui {
    conf.commands += echo \"$$LITERAL_HASH define MESHESDIR \\\"$$INSTALLROOT/$$MESHESDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define COLORFILTERSDIR \\\"$$INSTALLROOT/$$COLORFILTERSDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define USERCOLORFILTERSDIR \\\"$$USERCOLORFILTERSDIR\\\"\" >> $$CONFIGFILE &&
}
    conf.commands += echo \"$$LITERAL_HASH endif\" >> $$CONFIGFILE
}


# in case of a shadow build, copy CONFIGFILE back
# to the original QLC+ source tree
!equals(PWD, $$OUT_PWD) {
    message("Shadow build on")
    shadow.target = $$PWD/$$CONFIGFILE
    shadow.commands = $(COPY) $$OUT_PWD/$$CONFIGFILE $$PWD
    QMAKE_EXTRA_TARGETS += shadow
    PRE_TARGETDEPS += $$PWD/$$CONFIGFILE
    QMAKE_CLEAN += $$PWD/$$CONFIGFILE
    QMAKE_DISTCLEAN += $$PWD/$$CONFIGFILE
}
