include(variables.pri)

TEMPLATE = subdirs

SUBDIRS      += hotplugmonitor
SUBDIRS      += engine
SUBDIRS      += ui
SUBDIRS      += main
SUBDIRS      += fixtures
SUBDIRS      += inputprofiles
SUBDIRS      += rgbscripts
SUBDIRS      += fixtureeditor
SUBDIRS      += etc
SUBDIRS      += docs
macx:SUBDIRS += launcher
SUBDIRS      += plugins

unix:!macx:DEBIAN_CLEAN    += debian/*.substvars debian/*.log debian/*.debhelper
unix:!macx:DEBIAN_CLEAN    += debian/files debian/dirs
unix:!macx:QMAKE_CLEAN     += $$DEBIAN_CLEAN
unix:!macx:QMAKE_DISTCLEAN += $$DEBIAN_CLEAN

# Unit testing thru "make check"
unittests.target = check
QMAKE_EXTRA_TARGETS += unittests
unix:unittests.commands += ./unittest.sh
win32:unittests.commands += unittest.bat

# Unit test coverage measurement
coverage.target = lcov
QMAKE_EXTRA_TARGETS += coverage
unix:coverage.commands += ./coverage.sh
win32:coverage.commands = @echo Get a better OS.

# Translations (update these also to translate.sh and translate.bat)
translations.files += qlc_de_DE.qm qlc_es_ES.qm qlc_fi_FI.qm qlc_fr_FR.qm qlc_it_IT.qm
translations.path   = $$INSTALLROOT/$$TRANSLATIONDIR
INSTALLS           += translations

# Leave this on the last row of this file
macx:SUBDIRS += macx
