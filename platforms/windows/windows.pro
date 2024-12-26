include(../../variables.pri)

TEMPLATE = subdirs

greaterThan(QT_MAJOR_VERSION, 5) {
    QT_V="Qt6"
	QT_P="qt6"
} else {
	QT_V="Qt5"
	QT_P="qt5"
}
QT_D=""
debug: QT_D="d"

QT_LIBS_PATH = $$dirname(QMAKE_QMAKE)
QT_PLUGINS_PATH = $$QT_LIBS_PATH/../share/$${QT_P}/plugins
QT_QML_PATH = $$QT_LIBS_PATH/../share/$${QT_P}/qml
SYS_LIBS_PATH = $$(SystemDrive)/msys64/mingw32/bin
#SYS_LIBS_PATH = D:/msys64/mingw32/bin

# Qt Libraries
qtlibs.path  = $$INSTALLROOT/$$LIBSDIR

qtlibs.files = $$QT_LIBS_PATH/$${QT_V}Core$${QT_D}.dll \
               $$QT_LIBS_PATH/$${QT_V}Script$${QT_D}.dll \
               $$QT_LIBS_PATH/$${QT_V}Network$${QT_D}.dll \
               $$QT_LIBS_PATH/$${QT_V}Gui$${QT_D}.dll \
               $$QT_LIBS_PATH/$${QT_V}Svg$${QT_D}.dll \
               $$QT_LIBS_PATH/$${QT_V}Widgets$${QT_D}.dll \
               $$QT_LIBS_PATH/$${QT_V}OpenGL$${QT_D}.dll \
               $$QT_LIBS_PATH/$${QT_V}Multimedia$${QT_D}.dll \
               $$QT_LIBS_PATH/$${QT_V}MultimediaWidgets$${QT_D}.dll \
               $$QT_LIBS_PATH/$${QT_V}SerialPort$${QT_D}.dll \
               $$QT_LIBS_PATH/$${QT_V}WebSockets$${QT_D}.dll
greaterThan(QT_MAJOR_VERSION, 5) {
qtlibs.files += $$QT_LIBS_PATH/$${QT_V}Qml$${QT_D}.dll
}

qmlui: {
    qtlibs.files += $$QT_LIBS_PATH/$${QT_V}Qml$${QT_D}.dll \
                    $$QT_LIBS_PATH/$${QT_V}QmlModels$${QT_D}.dll \
                    $$QT_LIBS_PATH/$${QT_V}QmlWorkerScript$${QT_D}.dll \
                    $$QT_LIBS_PATH/$${QT_V}Quick$${QT_D}.dll \
                    $$QT_LIBS_PATH/$${QT_V}QuickControls2$${QT_D}.dll \
                    $$QT_LIBS_PATH/$${QT_V}QuickTemplates2$${QT_D}.dll \
                    $$QT_LIBS_PATH/$${QT_V}Sql$${QT_D}.dll \
                    $$QT_LIBS_PATH/$${QT_V}3DCore$${QT_D}.dll \
                    $$QT_LIBS_PATH/$${QT_V}3DExtras$${QT_D}.dll \
                    $$QT_LIBS_PATH/$${QT_V}3DInput$${QT_D}.dll \
                    $$QT_LIBS_PATH/$${QT_V}3DLogic$${QT_D}.dll \
                    $$QT_LIBS_PATH/$${QT_V}3DAnimation$${QT_D}.dll \
                    $$QT_LIBS_PATH/$${QT_V}3DQuick$${QT_D}.dll \
                    $$QT_LIBS_PATH/$${QT_V}3DQuickExtras$${QT_D}.dll \
                    $$QT_LIBS_PATH/$${QT_V}3DQuickInput$${QT_D}.dll \
                    $$QT_LIBS_PATH/$${QT_V}3DQuickAnimation$${QT_D}.dll \
                    $$QT_LIBS_PATH/$${QT_V}3DQuickRender$${QT_D}.dll \
                    $$QT_LIBS_PATH/$${QT_V}3DRender$${QT_D}.dll \
                    $$QT_LIBS_PATH/$${QT_V}Concurrent$${QT_D}.dll \
                    $$QT_LIBS_PATH/$${QT_V}Gamepad$${QT_D}.dll \
                    $$QT_LIBS_PATH/$${QT_V}PrintSupport$${QT_D}.dll \
                    $$QT_LIBS_PATH/$${QT_V}MultimediaQuick$${QT_D}.dll
}

# Qt library dependencies
qtdeps.path  = $$INSTALLROOT/$$LIBSDIR

qtdeps.files += $$SYS_LIBS_PATH/libbz2-1.dll \
                $$SYS_LIBS_PATH/libfreetype-6.dll \
                $$SYS_LIBS_PATH/libglib-2.0-0.dll \
                $$SYS_LIBS_PATH/libgraphite2.dll \
                $$SYS_LIBS_PATH/libharfbuzz-0.dll \
                $$SYS_LIBS_PATH/libdouble-conversion.dll \
                $$SYS_LIBS_PATH/libiconv-2.dll \
                $$SYS_LIBS_PATH/libintl-8.dll \
                $$SYS_LIBS_PATH/libpcre2-8-0.dll \
                $$SYS_LIBS_PATH/libpcre2-16-0.dll \
                $$SYS_LIBS_PATH/libpcre-1.dll \
                $$SYS_LIBS_PATH/libpng16-16.dll \
                $$SYS_LIBS_PATH/libjpeg-8.dll \
                $$SYS_LIBS_PATH/libspeex-1.dll \
                $$SYS_LIBS_PATH/libzstd.dll \
                $$SYS_LIBS_PATH/libbrotlidec.dll \
                $$SYS_LIBS_PATH/libbrotlicommon.dll \
                $$SYS_LIBS_PATH/zlib1.dll
greaterThan(QT_MAJOR_VERSION, 5) {
qtdeps.files += $$SYS_LIBS_PATH/libb2-1.dll
}

qmlui: {
    qtdeps.files += $$SYS_LIBS_PATH/libassimp-5.dll \
                    $$SYS_LIBS_PATH/libminizip-1.dll
}
INSTALLS += qtdeps

INSTALLS += qtlibs

qtplatform.path = $$INSTALLROOT/$$LIBSDIR/platforms
qtplatform.files = $$QT_PLUGINS_PATH/platforms/qwindows$${QT_D}.dll
INSTALLS += qtplatform

qtstyles.path = $$INSTALLROOT/$$LIBSDIR/styles
qtstyles.files = $$QT_PLUGINS_PATH/styles/qwindowsvistastyle$${QT_D}.dll
INSTALLS += qtstyles

greaterThan(QT_MAJOR_VERSION, 5) {
	qtmedia.path = $$INSTALLROOT/$$LIBSDIR/multimedia
	qtmedia.files = $$QT_PLUGINS_PATH/multimedia/ffmpegmediaplugin$${QT_D}.dll \
					$$QT_PLUGINS_PATH/multimedia/windowsmediaplugin$${QT_D}.dll
	INSTALLS += qtmedia
} else {
	qtaudio.path = $$INSTALLROOT/$$LIBSDIR/audio
	qtaudio.files = $$QT_PLUGINS_PATH/audio/qtaudio_windows$${QT_D}.dll
	INSTALLS += qtaudio

	qtmedia.path = $$INSTALLROOT/$$LIBSDIR/mediaservice
	qtmedia.files = $$QT_PLUGINS_PATH/mediaservice/dsengine$${QT_D}.dll \
					$$QT_PLUGINS_PATH/mediaservice/qtmedia_audioengine$${QT_D}.dll
	INSTALLS += qtmedia
}

qtimageformats.path = $$INSTALLROOT/$$LIBSDIR/imageformats
qtimageformats.files = $$QT_PLUGINS_PATH/imageformats/qgif$${QT_D}.dll \
                       $$QT_PLUGINS_PATH/imageformats/qjpeg$${QT_D}.dll \
                       $$QT_PLUGINS_PATH/imageformats/qsvg$${QT_D}.dll
INSTALLS += qtimageformats

qmlui: {
    qtprintsupport.path = $$INSTALLROOT/$$LIBSDIR/printsupport
    qtprintsupport.files = $$QT_PLUGINS_PATH/printsupport/windowsprintersupport$${QT_D}.dll
    INSTALLS += qtprintsupport

    geometryloaders.path = $$INSTALLROOT/$$LIBSDIR/geometryloaders
    geometryloaders.files = $$QT_PLUGINS_PATH/geometryloaders/defaultgeometryloader$${QT_D}.dll
    INSTALLS += geometryloaders

    sceneparsers.path = $$INSTALLROOT/$$LIBSDIR/sceneparsers
    sceneparsers.files = $$QT_PLUGINS_PATH/sceneparsers/assimpsceneimport$${QT_D}.dll
    INSTALLS += sceneparsers

    renderers.path = $$INSTALLROOT/$$LIBSDIR/renderers
    renderers.files = $$QT_PLUGINS_PATH/renderers/openglrenderer$${QT_D}.dll
    INSTALLS += renderers

    qmldeps.path   = $$INSTALLROOT/$$LIBSDIR
    qmldeps.files += $$QT_QML_PATH/Qt \
                     $$QT_QML_PATH/QtQml \
                     $$QT_QML_PATH/QtQuick \
                     $$QT_QML_PATH/QtQuick.2 \
                     $$QT_QML_PATH/Qt3D \
                     $$QT_QML_PATH/QtMultimedia

    INSTALLS += qmldeps

    qmlpostinstall.path = $$INSTALLROOT/$$LIBSDIR
    qmlpostinstall.commands = cd $$INSTALLROOT/$$LIBSDIR && \
                              find . -name plugins.qmltypes -type f -delete && \
                              find . -name *.qmlc -type f -delete && \
                              rm -rf Qt/WebSockets Qt/labs/location QtQml/RemoteObjects \
                              rm -rf QtQuick/Extras QtQuick/Particles.2 QtQuick/XmlListModel \
                              rm -rf QtQuick/Controls.2/designer QtQuick/Controls.2/Material \
                              rm -rf QtQuick/Controls.2/Universal QtQuick/Controls.2/Fusion \
                              rm -rf QtQuick/Controls.2/Imagine QtQuick/Scene2D
    INSTALLS  += qmlpostinstall
}

# MSYS2 libraries
msys.path = $$INSTALLROOT/$$LIBSDIR
msys.files += $$SYS_LIBS_PATH/libstdc++-6.dll
msys.files += $$SYS_LIBS_PATH/libgcc_s_dw2-1.dll
msys.files += $$SYS_LIBS_PATH/libwinpthread-1.dll
msys.files += $$SYS_LIBS_PATH/libicuin74.dll
msys.files += $$SYS_LIBS_PATH/libicuuc74.dll
msys.files += $$SYS_LIBS_PATH/libicudt74.dll
msys.files += $$SYS_LIBS_PATH/libmd4c.dll
msys.files += $$SYS_LIBS_PATH/libusb-1.0.dll

INSTALLS += msys

# audio libraries
audio.path = $$INSTALLROOT/$$LIBSDIR
exists($$SYS_LIBS_PATH/libmad-0.dll) {
    audio.files += $$SYS_LIBS_PATH/libmad-0.dll
}
exists($$SYS_LIBS_PATH/libogg-0.dll) {
    audio.files += $$SYS_LIBS_PATH/libogg-0.dll
}
exists($$SYS_LIBS_PATH/libopus-0.dll) {
    audio.files += $$SYS_LIBS_PATH/libopus-0.dll
}
exists($$SYS_LIBS_PATH/libmp3lame-0.dll) {
    audio.files += $$SYS_LIBS_PATH/libmp3lame-0.dll
}
exists($$SYS_LIBS_PATH/libmpg123-0.dll) {
    audio.files += $$SYS_LIBS_PATH/libmpg123-0.dll
}
exists($$SYS_LIBS_PATH/libvorbis-0.dll) {
    audio.files += $$SYS_LIBS_PATH/libvorbis-0.dll
}
exists($$SYS_LIBS_PATH/libvorbisenc-2.dll) {
    audio.files += $$SYS_LIBS_PATH/libvorbisenc-2.dll
}
exists($$SYS_LIBS_PATH/libFLAC.dll) {
    audio.files += $$SYS_LIBS_PATH/libFLAC.dll
}
exists($$SYS_LIBS_PATH/libsndfile-1.dll) {
    audio.files += $$SYS_LIBS_PATH/libsndfile-1.dll
}
exists($$SYS_LIBS_PATH/libfftw3-3.dll) {
    audio.files += $$SYS_LIBS_PATH/libfftw3-3.dll
}

INSTALLS += audio

# NullSoft installer files
nsis.path  = $$INSTALLROOT/$$DATADIR

qmlui: {
    nsis.files = qlcplus5$${QT_V}.nsi
} else {
    nsis.files = qlcplus4$${QT_V}.nsi
}

INSTALLS  += nsis

samples.files += ../Sample.qxw
samples.path   = $$INSTALLROOT/$$DATADIR
INSTALLS      += samples
