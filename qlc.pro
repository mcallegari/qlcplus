include(variables.pri)

TEMPLATE = subdirs

SUBDIRS        += hotplugmonitor
SUBDIRS        += engine

qmlui: {
  message("Building QLC+ 5 QML UI")
  SUBDIRS      += qmlui
} else {
  message("Building QLC+ 4 QtWidget UI")
  SUBDIRS      += ui
  SUBDIRS      += webaccess
  SUBDIRS      += main
  SUBDIRS      += fixtureeditor
  macx:SUBDIRS += launcher
}
SUBDIRS        += resources
SUBDIRS        += plugins

unix:!macx:DEBIAN_CLEAN    += debian/*.substvars debian/*.log debian/*.debhelper
unix:!macx:DEBIAN_CLEAN    += debian/files debian/dirs
unix:!macx:QMAKE_CLEAN     += $$DEBIAN_CLEAN
unix:!macx:QMAKE_DISTCLEAN += $$DEBIAN_CLEAN

# Unit testing thru "make check"
unittests.target = check
QMAKE_EXTRA_TARGETS += unittests
qmlui: {
  unix:unittests.commands += ./unittest.sh "qmlui"
  win32:unittests.commands += unittest.bat "qmlui"
} else {
  unix:unittests.commands += ./unittest.sh "ui"
  win32:unittests.commands += unittest.bat "ui"
}

# Unit test coverage measurement
coverage.target = lcov
QMAKE_EXTRA_TARGETS += coverage
qmlui: {
unix:coverage.commands += ./coverage.sh "qmlui"
} else {
unix:coverage.commands += ./coverage.sh "ui"
}
win32:coverage.commands = @echo Get a better OS.

# Translations
translations.target = translate
!android: {
  QMAKE_EXTRA_TARGETS += translations
}
qmlui: {
  translations.commands += ./translate.sh "qmlui"
} else {
  translations.commands += ./translate.sh "ui"
}
translations.files = *.qm
appimage: {
  translations.path   = $$TARGET_DIR/$$INSTALLROOT/$$TRANSLATIONDIR
} else {
  translations.path   = $$INSTALLROOT/$$TRANSLATIONDIR
}
!android: {
  INSTALLS           += translations
}
QMAKE_DISTCLEAN += $$translations.files

# run
run.target = run
QMAKE_EXTRA_TARGETS += run
qmlui: {
unix:run.commands += LD_LIBRARY_PATH=engine/src:\$\$LD_LIBRARY_PATH qmlui/qlcplus-qml
} else {
unix:run.commands += LD_LIBRARY_PATH=engine/src:ui/src:webaccess/src:\$\$LD_LIBRARY_PATH main/qlcplus
}

# run-fxe
run-fxe.target = run-fxe
QMAKE_EXTRA_TARGETS += run-fxe
qmlui: {
} else {
unix:run-fxe.commands += LD_LIBRARY_PATH=engine/src:ui/src:webaccess/src:\$\$LD_LIBRARY_PATH ./fixtureeditor/qlcplus-fixtureeditor
}

# doxygen
doxygen.target = doxygen
QMAKE_EXTRA_TARGETS += doxygen
unix:doxygen.commands += cd resources/doxygen && rm -rf html/ && doxygen qlcplus.dox

# Leave this on the last row of this file
SUBDIRS += platforms
