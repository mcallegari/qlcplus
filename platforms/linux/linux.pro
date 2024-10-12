include(../../variables.pri)

TEMPLATE = subdirs
TARGET   = icons

desktop.path   = $$INSTALLROOT/share/applications/
desktop.files += qlcplus.desktop
!qmlui: {
    desktop.files += qlcplus-fixtureeditor.desktop
}
INSTALLS      += desktop

icons.path   = $$INSTALLROOT/share/pixmaps/
icons.files += ../../resources/icons/png/qlcplus.png
!qmlui: {
    icons.files += ../../resources/icons/png/qlcplus-fixtureeditor.png
}
INSTALLS    += icons

mime.path   = $$INSTALLROOT/share/mime/packages
mime.files += qlcplus.xml
INSTALLS   += mime

appdata.path   = $$METAINFODIR
appdata.files += org.qlcplus.QLCPlus.appdata.xml
!qmlui: {
    appdata.files += org.qlcplus.QLCPlusFixtureEditor.appdata.xml
}
INSTALLS      += appdata

!qmlui: {
manpages.path = $$INSTALLROOT/$$MANDIR
manpages.files += *.1
INSTALLS += manpages
}

# This is nowadays run by dpkg (TODO: rpm)
#MIMEUPDATE    = $$system("which update-mime-database")
#mimeupdate.commands = $$MIMEUPDATE /usr/share/mime
#mimeupdate.path = /usr/share/mime
#INSTALLS += mimeupdate

samples.files += ../Sample.qxw
samples.path   = $$INSTALLROOT/$$DATADIR
INSTALLS      += samples

appimage: {
    QT_LIBS_PATH = $$dirname(QMAKE_QMAKE)/../lib
    QT_PLUGINS_PATH = $$QT_LIBS_PATH/../plugins
    QT_QML_PATH = $$QT_LIBS_PATH/../qml

    # Qt dependencies
    qtdeps.path = $$INSTALLROOT/$$LIBSDIR
    qtdeps.files = $$QT_LIBS_PATH/libicu*
    INSTALLS += qtdeps

    # Qt Libraries
    qtlibs.path  = $$INSTALLROOT/$$LIBSDIR
    qtlibs.files += $$QT_LIBS_PATH/libQt5Core.so.5 \
                    $$QT_LIBS_PATH/libQt5Script.so.5 \
                    $$QT_LIBS_PATH/libQt5Network.so.5 \
                    $$QT_LIBS_PATH/libQt5Gui.so.5 \
                    $$QT_LIBS_PATH/libQt5Svg.so.5 \
                    $$QT_LIBS_PATH/libQt5Widgets.so.5 \
                    $$QT_LIBS_PATH/libQt5OpenGL.so.5 \
                    $$QT_LIBS_PATH/libQt5Multimedia.so.5 \
                    $$QT_LIBS_PATH/libQt5MultimediaWidgets.so.5 \
                    $$QT_LIBS_PATH/libQt5SerialPort.so.5 \
                    $$QT_LIBS_PATH/libQt5XcbQpa.so.5 \
                    $$QT_LIBS_PATH/libQt5DBus.so.5 \
                    $$QT_LIBS_PATH/libQt5WebSockets.so.5
qmlui: {
    qtlibs.files += $$QT_LIBS_PATH/libQt5MultimediaQuick.so.5 \
                    $$QT_LIBS_PATH/libQt5MultimediaGstTools.so.5 \
                    $$QT_LIBS_PATH/libQt5Qml.so.5 \
                    $$QT_LIBS_PATH/libQt5QmlModels.so.5 \
                    $$QT_LIBS_PATH/libQt5QmlWorkerScript.so.5 \
                    $$QT_LIBS_PATH/libQt5Quick.so.5 \
                    $$QT_LIBS_PATH/libQt5QuickControls2.so.5 \
                    $$QT_LIBS_PATH/libQt5QuickTemplates2.so.5 \
                    $$QT_LIBS_PATH/libQt53DCore.so.5 \
                    $$QT_LIBS_PATH/libQt53DExtras.so.5 \
                    $$QT_LIBS_PATH/libQt53DInput.so.5 \
                    $$QT_LIBS_PATH/libQt53DLogic.so.5 \
                    $$QT_LIBS_PATH/libQt53DAnimation.so.5 \
                    $$QT_LIBS_PATH/libQt53DQuick.so.5 \
                    $$QT_LIBS_PATH/libQt53DQuickExtras.so.5 \
                    $$QT_LIBS_PATH/libQt53DQuickInput.so.5 \
                    $$QT_LIBS_PATH/libQt53DQuickRender.so.5 \
                    $$QT_LIBS_PATH/libQt53DRender.so.5 \
                    $$QT_LIBS_PATH/libQt5Concurrent.so.5 \
                    $$QT_LIBS_PATH/libQt5Gamepad.so.5 \
                    $$QT_LIBS_PATH/libQt5PrintSupport.so.5
}

    INSTALLS += qtlibs

    # Qt plugins
    qtplatform.path = $$INSTALLROOT/$$LIBSDIR/qt5/plugins/platforms
    qtplatform.files = $$QT_PLUGINS_PATH/platforms/libqlinuxfb.so \
                       $$QT_PLUGINS_PATH/platforms/libqxcb.so \
                       $$QT_PLUGINS_PATH/platforms/libqminimal.so
    INSTALLS += qtplatform

    qtxcbgl.path = $$INSTALLROOT/$$LIBSDIR/qt5/plugins/xcbglintegrations
    qtxcbgl.files = $$QT_PLUGINS_PATH/xcbglintegrations/libqxcb-glx-integration.so
    INSTALLS += qtxcbgl

    qtaudio.path = $$INSTALLROOT/$$LIBSDIR/qt5/plugins/audio
    qtaudio.files = $$QT_PLUGINS_PATH/audio/libqtaudio_alsa.so \
                    $$QT_PLUGINS_PATH/audio/libqtmedia_pulse.so
    INSTALLS += qtaudio

    qtmedia.path = $$INSTALLROOT/$$LIBSDIR/qt5/plugins/mediaservice
    qtmedia.files = $$QT_PLUGINS_PATH/mediaservice/libgstaudiodecoder.so \
                    $$QT_PLUGINS_PATH/mediaservice/libgstmediaplayer.so
    INSTALLS += qtmedia

    qtimageformats.path = $$INSTALLROOT/$$LIBSDIR/qt5/plugins/imageformats
    qtimageformats.files = $$QT_PLUGINS_PATH/imageformats/libqsvg.so
    INSTALLS += qtimageformats

qmlui: {
    qtprintsupport.path = $$INSTALLROOT/$$LIBSDIR/qt5/plugins/printsupport
    qtprintsupport.files = $$QT_PLUGINS_PATH/printsupport/libcupsprintersupport.so
    INSTALLS += qtprintsupport

    sceneparsers.path = $$INSTALLROOT/$$LIBSDIR/qt5/plugins/sceneparsers
    sceneparsers.files = $$QT_PLUGINS_PATH/sceneparsers/libassimpsceneimport.so
    INSTALLS += sceneparsers

    geometryloaders.path = $$INSTALLROOT/$$LIBSDIR/qt5/plugins/geometryloaders
    geometryloaders.files = $$QT_PLUGINS_PATH/geometryloaders/libdefaultgeometryloader.so
    INSTALLS += geometryloaders
}

versionAtLeast(QT_VERSION, 5.15.0) {
    renderers.path = $$INSTALLROOT/$$LIBSDIR/qt5/plugins/renderers
    renderers.files = $$QT_PLUGINS_PATH/renderers/libopenglrenderer.so
    INSTALLS += renderers
}

    qmldeps.path   = $$INSTALLROOT/bin
    qmldeps.files += $$QT_QML_PATH/QtQml \
                     $$QT_QML_PATH/QtQuick \
                     $$QT_QML_PATH/QtQuick.2 \
                     $$QT_QML_PATH/Qt3D \
                     $$QT_QML_PATH/QtMultimedia

    INSTALLS += qmldeps
}
