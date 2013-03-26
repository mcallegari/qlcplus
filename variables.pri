#############################################################################
# Application name & version
#############################################################################

APPNAME    = Q Light Controller Plus
FXEDNAME   = Fixture Definition Editor
APPVERSION = 4.3.2

#############################################################################
# Compiler & linker configuration
#############################################################################

# Treat all compiler warnings as errors
QMAKE_CXXFLAGS += -Werror
CONFIG         += warn_on

# Build everything in the order specified in .pro files
CONFIG         += ordered

CONFIG         -= release # Enable this when making a release
CONFIG         += debug   # Disable this when making a release

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

# Binaries
win32:BINDIR      =
unix:!macx:BINDIR = bin
macx:BINDIR       = MacOS

# Libraries
win32:LIBSDIR      =
unix:!macx:LIBSDIR = lib
macx:LIBSDIR       = Frameworks

# Data
win32:DATADIR      =
unix:!macx:DATADIR = share/qlcplus
macx:DATADIR       = Resources

# Documentation
win32:DOCSDIR      = Documents
unix:!macx:DOCSDIR = $$DATADIR/documents
macx:DOCSDIR       = $$DATADIR/Documents

# Input profiles
win32:INPUTPROFILEDIR      = InputProfiles
unix:!macx:INPUTPROFILEDIR = $$DATADIR/inputprofiles
macx:INPUTPROFILEDIR       = $$DATADIR/InputProfiles

# User input profiles
win32:USERINPUTPROFILEDIR      = QLC+/InputProfiles
unix:!macx:USERINPUTPROFILEDIR = .qlcplus/inputprofiles
macx:USERINPUTPROFILEDIR       = "Library/Application Support/QLC+/InputProfiles"

# Fixtures
win32:FIXTUREDIR      = Fixtures
unix:!macx:FIXTUREDIR = $$DATADIR/fixtures
macx:FIXTUREDIR       = $$DATADIR/Fixtures

# Gobos
win32:GOBODIR      = Gobos
unix:!macx:GOBODIR = $$DATADIR/gobos
macx:GOBODIR       = $$DATADIR/Gobos

# User fixtures
win32:USERFIXTUREDIR      = QLC+/Fixtures
unix:!macx:USERFIXTUREDIR = .qlcplus/fixtures
macx:USERFIXTUREDIR       = "Library/Application Support/QLC+/Fixtures"

# Plugins
win32:PLUGINDIR      = Plugins
unix:!macx:PLUGINDIR = $$LIBSDIR/qt4/plugins/qlcplus
macx:PLUGINDIR       = Plugins

# Translations
win32:TRANSLATIONDIR      =
unix:!macx:TRANSLATIONDIR = $$DATADIR/translations
macx:TRANSLATIONDIR       = $$DATADIR/Translations

# RGB Scripts
win32:RGBSCRIPTDIR      = RGBScripts
unix:!macx:RGBSCRIPTDIR = $$DATADIR/rgbscripts
macx:RGBSCRIPTDIR       = $$DATADIR/RGBScripts

# User RGB Scripts
win32:USERRGBSCRIPTDIR      = RGBScripts
unix:!macx:USERRGBSCRIPTDIR = .qlcplus/rgbscripts
macx:USERRGBSCRIPTDIR       = "Library/Application Support/QLC+/RGBScripts"
