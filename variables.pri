#############################################################################
# Application name & version
#############################################################################

APPNAME    = Q Light Controller Plus
FXEDNAME   = Fixture Definition Editor
!qmlui: APPVERSION = 4.14.0
qmlui:  APPVERSION = 5.0.0 GIT

# Disable these if you don't want to see GIT short hash in the About Box
#unix:REVISION = $$system(git log --pretty=format:'%h' -n 1)
#unix:APPVERSION = $$APPVERSION-r$$REVISION

#############################################################################
# Compiler & linker configuration
#############################################################################

# Treat all compiler warnings as errors
QMAKE_CXXFLAGS += -Werror
unix:QMAKE_CFLAGS += -Werror

CONFIG         += warn_on

# Mobile platforms are QML only
android|ios: CONFIG += qmlui

# Build everything in the order specified in .pro files
CONFIG         += ordered

qmlui: DEFINES += QMLUI

contains(FORCECONFIG, release) {
  message("Forcing a release build")
  CONFIG += release
  CONFIG -= debug
  #DEFINES += QT_NO_DEBUG_OUTPUT
} else {
  # Enable the following 2 lines when making a release
  CONFIG         += release
  DEFINES        += QT_NO_DEBUG_OUTPUT

  # Disable this when making a release
  CONFIG         -= debug
}

!macx:!ios: {
 system( g++ --version | grep -e "4.6.[0-9]" ) {
   #message("g++ version 4.6 found")
   QMAKE_CXXFLAGS += -Wno-error=strict-overflow
 }
 else {
   QMAKE_CXXFLAGS += -Wno-unused-local-typedefs # Fix to build with GCC 4.8
   QMAKE_CXXFLAGS += -Wno-template-id-cdtor # Fix to build with GCC 14
 }
}

unix:OLA_GIT    = /usr/src/ola    # OLA directories

#macx:CONFIG   += x86 ppc  # Build universal binaries (Leopard only)
macx:CONFIG    -= app_bundle # Let QLC+ construct the .app bundle
macx:QMAKE_STRIP = strip -x
macx:QMAKE_LFLAGS += -Wl,-rpath,@executable_path/../Frameworks

# Produce build targets to the source directory
win32:DESTDIR  = ./

# Don't whine about some imports
win32:QMAKE_LFLAGS += -Wl,--enable-auto-import

# Enable unit test coverage measurement ('qmake CONFIG+=coverage' works, too)
#CONFIG        += coverage

#############################################################################
# Installation paths
#############################################################################

# Install root
win32:INSTALLROOT       = $$(SystemDrive)/qlcplus
macx:INSTALLROOT        = ~/QLC+.app/Contents
unix:!macx:INSTALLROOT += /usr
android:INSTALLROOT     = /
ios:INSTALLROOT         = /

# Binaries
win32:BINDIR      =
unix:!macx:BINDIR = bin
macx:BINDIR       = MacOS
android:BINDIR    = bin
ios:BINDIR        =

# Libraries
win32:LIBSDIR      =
unix:!macx:LIBSDIR = lib/x86_64-linux-gnu
macx:LIBSDIR       = Frameworks
android:LIBSDIR    = /libs/armeabi-v7a
ios:LIBSDIR        = lib

# Data
win32:DATADIR      =
unix:!macx:DATADIR = share/qlcplus
macx:DATADIR       = Resources
android:DATADIR    = /assets
ios:DATADIR        =
appimage: DATADIR  = ../share/qlcplus

# User Data
win32:USERDATADIR      = QLC+
unix:!macx:USERDATADIR = .qlcplus
macx:USERDATADIR       = "Library/Application Support/QLC+"
android:USERDATADIR    = .qlcplus
ios:USERDATADIR        = .qlcplus

# Documentation
win32:DOCSDIR      = Documents
unix:!macx:DOCSDIR = $$DATADIR/documents
macx:DOCSDIR       = $$DATADIR/Documents
android:DOCSDIR    = $$DATADIR/documents
ios:DOCSDIR        = Documents

# Input profiles
win32:INPUTPROFILEDIR      = InputProfiles
unix:!macx:INPUTPROFILEDIR = $$DATADIR/inputprofiles
macx:INPUTPROFILEDIR       = $$DATADIR/InputProfiles
android:INPUTPROFILEDIR    = $$DATADIR/inputprofiles
ios:INPUTPROFILEDIR        = InputProfiles

# User input profiles
win32:USERINPUTPROFILEDIR      = $$USERDATADIR/InputProfiles
unix:!macx:USERINPUTPROFILEDIR = $$USERDATADIR/inputprofiles
macx:USERINPUTPROFILEDIR       = $$USERDATADIR/InputProfiles
android:USERINPUTPROFILEDIR    = $$USERDATADIR/inputprofiles
ios:USERINPUTPROFILEDIR        = $$USERDATADIR/InputProfiles

# Midi templates
win32:MIDITEMPLATEDIR      = MidiTemplates
unix:!macx:MIDITEMPLATEDIR = $$DATADIR/miditemplates
macx:MIDITEMPLATEDIR       = $$DATADIR/MidiTemplates
android:MIDITEMPLATEDIR    = $$DATADIR/miditemplates
ios:MIDITEMPLATEDIR        = MidiTemplates

# User midi templates
win32:USERMIDITEMPLATEDIR      = $$USERDATADIR/MidiTemplates
unix:!macx:USERMIDITEMPLATEDIR = $$USERDATADIR/miditemplates
macx:USERMIDITEMPLATEDIR       = $$USERDATADIR/MidiTemplates
android:USERMIDITEMPLATEDIR    = $$USERDATADIR/miditemplates
ios:USERMIDITEMPLATEDIR        = $$USERDATADIR/MidiTemplates

# Channel modifiers templates
win32:MODIFIERSTEMPLATEDIR      = ModifiersTemplates
unix:!macx:MODIFIERSTEMPLATEDIR = $$DATADIR/modifierstemplates
macx:MODIFIERSTEMPLATEDIR       = $$DATADIR/ModifiersTemplates
android:MODIFIERSTEMPLATEDIR    = $$DATADIR/modifierstemplates
ios:MODIFIERSTEMPLATEDIR        = ModifiersTemplates

# User midi templates
win32:USERMODIFIERSTEMPLATEDIR      = $$USERDATADIR/ModifiersTemplates
unix:!macx:USERMODIFIERSTEMPLATEDIR = $$USERDATADIR/modifierstemplates
macx:USERMODIFIERSTEMPLATEDIR       = $$USERDATADIR/ModifiersTemplates
android:USERMODIFIERSTEMPLATEDIR    = $$USERDATADIR/modifierstemplates
ios:USERMODIFIERSTEMPLATEDIR        = $$USERDATADIR/ModifiersTemplates

# Fixtures
win32:FIXTUREDIR      = Fixtures
unix:!macx:FIXTUREDIR = $$DATADIR/fixtures
macx:FIXTUREDIR       = $$DATADIR/Fixtures
android:FIXTUREDIR    = $$DATADIR/fixtures
ios:FIXTUREDIR        = Fixtures

# Gobos
win32:GOBODIR      = Gobos
unix:!macx:GOBODIR = $$DATADIR/gobos
macx:GOBODIR       = $$DATADIR/Gobos
android:GOBODIR    = $$DATADIR/gobos
ios:GOBODIR        = Gobos

# User fixtures
win32:USERFIXTUREDIR      = $$USERDATADIR/Fixtures
unix:!macx:USERFIXTUREDIR = $$USERDATADIR/fixtures
macx:USERFIXTUREDIR       = $$USERDATADIR/Fixtures
android:USERFIXTUREDIR    = $$USERDATADIR/fixtures
ios:USERFIXTUREDIR        = $$USERDATADIR/Fixtures

# Plugins
win32:PLUGINDIR      = Plugins
unix:!macx:PLUGINDIR = $$LIBSDIR/qt5/plugins/qlcplus
macx:PLUGINDIR       = PlugIns
android:PLUGINDIR    = Plugins
ios:PLUGINDIR        = Plugins
appimage:PLUGINDIR   = ../lib/qt5/plugins/qlcplus

# Audio Plugins
win32:AUDIOPLUGINDIR      = $$PLUGINDIR/Audio
unix:!macx:AUDIOPLUGINDIR = $$PLUGINDIR/audio
macx:AUDIOPLUGINDIR       = $$PLUGINDIR/Audio
android:AUDIOPLUGINDIR    = $$PLUGINDIR/Audio
ios:AUDIOPLUGINDIR        = $$PLUGINDIR/Audio

# Translations
win32:TRANSLATIONDIR      =
unix:!macx:TRANSLATIONDIR = $$DATADIR/translations
macx:TRANSLATIONDIR       = $$DATADIR/Translations
android:TRANSLATIONDIR    = $$DATADIR/translations
ios:TRANSLATIONDIR        =

# RGB Scripts
win32:RGBSCRIPTDIR      = RGBScripts
unix:!macx:RGBSCRIPTDIR = $$DATADIR/rgbscripts
macx:RGBSCRIPTDIR       = $$DATADIR/RGBScripts
android:RGBSCRIPTDIR    = $$DATADIR/rgbscripts
ios:RGBSCRIPTDIR        = RGBScripts

# User RGB Scripts
win32:USERRGBSCRIPTDIR      = $$USERDATADIR/RGBScripts
unix:!macx:USERRGBSCRIPTDIR = $$USERDATADIR/rgbscripts
macx:USERRGBSCRIPTDIR       = $$USERDATADIR/RGBScripts
android:USERRGBSCRIPTDIR    = $$USERDATADIR/rgbscripts
ios:USERRGBSCRIPTDIR        = $$USERDATADIR/RGBScripts

# Web Files
win32:WEBFILESDIR      = Web
unix:!macx:WEBFILESDIR = $$DATADIR/web
macx:WEBFILESDIR       = $$DATADIR/Web
android:WEBFILESDIR    = $$DATADIR/web
ios:WEBFILESDIR        = Web

# Samples
win32:SAMPLESDIR       = $$INSTALLROOT
unix:!macx:SAMPLESDIR  = $$INSTALLROOT/$$DATADIR
macx:SAMPLESDIR        = $$INSTALLROOT/$$DATADIR
android:SAMPLESDIR     = $$INSTALLROOT/$$DATADIR
ios:SAMPLESDIR         = $$INSTALLROOT/$$DATADIR

# 3D Meshes
win32:MESHESDIR      = Meshes
unix:!macx:MESHESDIR = $$DATADIR/meshes
macx:MESHESDIR       = $$DATADIR/Meshes
android:MESHESDIR    = $$DATADIR/meshes
ios:MESHESDIR        = Meshes

# Color filters
win32:COLORFILTERSDIR      = ColorFilters
unix:!macx:COLORFILTERSDIR = $$DATADIR/colorfilters
macx:COLORFILTERSDIR       = $$DATADIR/ColorFilters
android:COLORFILTERSDIR    = $$DATADIR/colorfilters
ios:COLORFILTERSDIR        = ColorFilters

# User Color filters
win32:USERCOLORFILTERSDIR      = $$USERDATADIR/ColorFilters
unix:!macx:USERCOLORFILTERSDIR = $$USERDATADIR/colorfilters
macx:USERCOLORFILTERSDIR       = $$USERDATADIR/ColorFilters
android:USERCOLORFILTERSDIR    = $$USERDATADIR/colorfilters
ios:USERCOLORFILTERSDIR        = $$USERDATADIR/ColorFilters

# udev rules
unix:!macx:UDEVRULESDIR = /etc/udev/rules.d

# AppStream metadata
unix:!macx:METAINFODIR = $$INSTALLROOT/share/metainfo

# man
unix:!macx:MANDIR = share/man/man1/

unix:!macx: {
  QTPREFIX = $$[QT_INSTALL_PREFIX]
  IN_USR = $$find(QTPREFIX, "/usr")
  count(IN_USR, 1) {
    QTLIBSDIR = $$[QT_INSTALL_LIBS]
    QTPLUGINSDIR = $$[QT_INSTALL_PLUGINS]
    LIBSDIR = $$replace(QTLIBSDIR, "/usr/", "")
    PLUGINDIR = $$replace(QTPLUGINSDIR, "/usr/", "")/qlcplus
    AUDIOPLUGINDIR = $$PLUGINDIR/audio
  }
  #message("Qt install prefix: "$$[QT_INSTALL_PREFIX])
  #message("Qt install libs: "$$[QT_INSTALL_LIBS])
  #message("Linux libs dir: " $$INSTALLROOT/$$LIBSDIR)
  #message("Linux plugins dir: " $$INSTALLROOT/$$PLUGINDIR)
}
