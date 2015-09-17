#############################################################################
# Application name & version
#############################################################################

APPNAME    = Q Light Controller Plus
FXEDNAME   = Fixture Definition Editor
APPVERSION = 4.9.2 GIT

#############################################################################
# Compiler & linker configuration
#############################################################################

# Treat all compiler warnings as errors
QMAKE_CXXFLAGS += -Werror
!macx:QMAKE_CXXFLAGS += -Wno-unused-local-typedefs # Fix to build with GCC 4.8
CONFIG         += warn_on

# Build everything in the order specified in .pro files
CONFIG         += ordered

# Enable the following 2 lines when making a release
CONFIG         -= release
#DEFINES        += QT_NO_DEBUG_OUTPUT

# Disable this when making a release
CONFIG         += debug

# Disable these if you don't want to see SVN revision in the About Box
#unix:REVISION = $$system(svn info | grep "Revision" | sed 's/Revision://')
#unix:APPVERSION = $$APPVERSION-r$$REVISION

unix:OLA_GIT    = /usr/src/ola    # OLA directories

#macx:CONFIG   += x86 ppc  # Build universal binaries (Leopard only)
macx:CONFIG    -= app_bundle # Let QLC+ construct the .app bundle

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

# Binaries
win32:BINDIR      =
unix:!macx:BINDIR = bin
macx:BINDIR       = MacOS
android:BINDIR    = bin

# Libraries
win32:LIBSDIR      =
unix:!macx:LIBSDIR = lib
macx:LIBSDIR       = Frameworks
android:LIBSDIR    = /libs/armeabi-v7a

# Data
win32:DATADIR      =
unix:!macx:DATADIR = share/qlcplus
macx:DATADIR       = Resources
android:DATADIR    = /assets

# User Data
win32:USERDATADIR      = QLC+
unix:!macx:USERDATADIR = .qlcplus
macx:USERDATADIR       = "Library/Application Support/QLC+"
android:USERDATADIR    = .qlcplus

# Documentation
win32:DOCSDIR      = Documents
unix:!macx:DOCSDIR = $$DATADIR/Documents
macx:DOCSDIR       = $$DATADIR/Documents
android:DOCSDIR    = $$DATADIR/Documents

# Input profiles
win32:INPUTPROFILEDIR      = InputProfiles
unix:!macx:INPUTPROFILEDIR = $$DATADIR/Inputprofiles
macx:INPUTPROFILEDIR       = $$DATADIR/InputProfiles
android:INPUTPROFILEDIR    = $$DATADIR/Inputprofiles

# User input profiles
win32:USERINPUTPROFILEDIR      = $$USERDATADIR/InputProfiles
unix:!macx:USERINPUTPROFILEDIR = $$USERDATADIR/Inputprofiles
macx:USERINPUTPROFILEDIR       = $$USERDATADIR/InputProfiles
android:USERINPUTPROFILEDIR    = $$USERDATADIR/Inputprofiles

# Midi templates
win32:MIDITEMPLATEDIR      = MidiTemplates
unix:!macx:MIDITEMPLATEDIR = $$DATADIR/MidiTemplates
macx:MIDITEMPLATEDIR       = $$DATADIR/MidiTemplates
android:MIDITEMPLATEDIR    = $$DATADIR/MidiTemplates

# User midi templates
win32:USERMIDITEMPLATEDIR      = $$USERDATADIR/MidiTemplates
unix:!macx:USERMIDITEMPLATEDIR = $$USERDATADIR/MidiTemplates
macx:USERMIDITEMPLATEDIR       = $$USERDATADIR/MidiTemplates
android:USERMIDITEMPLATEDIR    = $$USERDATADIR/MidiTemplates

# Channel modifiers templates
win32:MODIFIERSTEMPLATEDIR      = ModifiersTemplates
unix:!macx:MODIFIERSTEMPLATEDIR = $$DATADIR/ModifiersTemplates
macx:MODIFIERSTEMPLATEDIR       = $$DATADIR/ModifiersTemplates
android:MODIFIERSTEMPLATEDIR    = $$DATADIR/ModifiersTemplates

# User midi templates
win32:USERMODIFIERSTEMPLATEDIR      = $$USERDATADIR/ModifiersTemplates
unix:!macx:USERMODIFIERSTEMPLATEDIR = $$USERDATADIR/ModifiersTemplates
macx:USERMODIFIERSTEMPLATEDIR       = $$USERDATADIR/ModifiersTemplates
android:USERMODIFIERSTEMPLATEDIR    = $$USERDATADIR/ModifiersTemplates

# Fixtures
win32:FIXTUREDIR      = Fixtures
unix:!macx:FIXTUREDIR = $$DATADIR/Fixtures
macx:FIXTUREDIR       = $$DATADIR/Fixtures
android:FIXTUREDIR    = $$DATADIR/Fixtures

# Gobos
win32:GOBODIR      = Gobos
unix:!macx:GOBODIR = $$DATADIR/Gobos
macx:GOBODIR       = $$DATADIR/Gobos
android:GOBODIR    = $$DATADIR/Gobos

# User fixtures
win32:USERFIXTUREDIR      = $$USERDATADIR/Fixtures
unix:!macx:USERFIXTUREDIR = $$USERDATADIR/Fixtures
macx:USERFIXTUREDIR       = $$USERDATADIR/Fixtures
android:USERFIXTUREDIR    = $$USERDATADIR/Fixtures

# Plugins
win32:PLUGINDIR      = Plugins
unix:!macx:PLUGINDIR = $$LIBSDIR/qt4/plugins/qlcplus
macx:PLUGINDIR       = Plugins
android:PLUGINDIR    = Plugins

# Translations
win32:TRANSLATIONDIR      =
unix:!macx:TRANSLATIONDIR = $$DATADIR/Translations
macx:TRANSLATIONDIR       = $$DATADIR/Translations
android:TRANSLATIONDIR    = $$DATADIR/Translations

# RGB Scripts
win32:RGBSCRIPTDIR      = RGBScripts
unix:!macx:RGBSCRIPTDIR = $$DATADIR/RGBscripts
macx:RGBSCRIPTDIR       = $$DATADIR/RGBScripts
android:RGBSCRIPTDIR    = $$DATADIR/RGBscripts

# User RGB Scripts
win32:USERRGBSCRIPTDIR      = $$USERDATADIR/RGBScripts
unix:!macx:USERRGBSCRIPTDIR = $$USERDATADIR/RGBscripts
macx:USERRGBSCRIPTDIR       = $$USERDATADIR/RGBScripts

# RGB Scripts
win32:WEBFILESDIR      = Web
unix:!macx:WEBFILESDIR = $$DATADIR/Web
macx:WEBFILESDIR       = $$DATADIR/Web
