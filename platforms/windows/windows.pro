include(../../variables.pri)

TEMPLATE = subdirs

QT_LIBS_PATH = $$dirname(QMAKE_QMAKE)
QT_PLUGINS_PATH = $$QT_LIBS_PATH/../share/qt5/plugins
QT_QML_PATH = $$QT_LIBS_PATH/../share/qt5/qml
SYS_LIBS_PATH = $$(SystemDrive)/msys64/mingw32/bin
#SYS_LIBS_PATH = D:/msys64/mingw32/bin
QT_D=""
debug: QT_D="d"

# Qt Libraries
qtlibs.path  = $$INSTALLROOT/$$LIBSDIR

lessThan(QT_MAJOR_VERSION, 5) {
    qtlibs.files = $$QT_LIBS_PATH/QtCore4$${QT_D}.dll \
                   $$QT_LIBS_PATH/QtGui4$${QT_D}.dll \
                   $$QT_LIBS_PATH/QtScript4$${QT_D}.dll \
                   $$QT_LIBS_PATH/QtNetwork4$${QT_D}.dll
} else {
    qtlibs.files = $$QT_LIBS_PATH/Qt5Core$${QT_D}.dll \
                   $$QT_LIBS_PATH/Qt5Script$${QT_D}.dll \
                   $$QT_LIBS_PATH/Qt5Network$${QT_D}.dll \
                   $$QT_LIBS_PATH/Qt5Gui$${QT_D}.dll \
                   $$QT_LIBS_PATH/Qt5Svg$${QT_D}.dll \
                   $$QT_LIBS_PATH/Qt5Widgets$${QT_D}.dll \
                   $$QT_LIBS_PATH/Qt5OpenGL$${QT_D}.dll \
                   $$QT_LIBS_PATH/Qt5Multimedia$${QT_D}.dll \
                   $$QT_LIBS_PATH/Qt5MultimediaWidgets$${QT_D}.dll

    qmlui: {
        qtlibs.files += $$QT_LIBS_PATH/Qt5Qml$${QT_D}.dll \
                        $$QT_LIBS_PATH/Qt5Quick$${QT_D}.dll \
                        $$QT_LIBS_PATH/Qt5QuickControls2$${QT_D}.dll \
                        $$QT_LIBS_PATH/Qt5QuickTemplates2$${QT_D}.dll \
                        $$QT_LIBS_PATH/Qt5Sql$${QT_D}.dll \
                        $$QT_LIBS_PATH/Qt53DCore$${QT_D}.dll \
                        $$QT_LIBS_PATH/Qt53DExtras$${QT_D}.dll \
                        $$QT_LIBS_PATH/Qt53DInput$${QT_D}.dll \
                        $$QT_LIBS_PATH/Qt53DLogic$${QT_D}.dll \
                        $$QT_LIBS_PATH/Qt53DAnimation$${QT_D}.dll \
                        $$QT_LIBS_PATH/Qt53DQuick$${QT_D}.dll \
                        $$QT_LIBS_PATH/Qt53DQuickExtras$${QT_D}.dll \
                        $$QT_LIBS_PATH/Qt53DQuickInput$${QT_D}.dll \
                        $$QT_LIBS_PATH/Qt53DQuickAnimation$${QT_D}.dll \
                        $$QT_LIBS_PATH/Qt53DQuickRender$${QT_D}.dll \
                        $$QT_LIBS_PATH/Qt53DRender$${QT_D}.dll \
                        $$QT_LIBS_PATH/Qt5Concurrent$${QT_D}.dll \
                        $$QT_LIBS_PATH/Qt5Gamepad$${QT_D}.dll \
                        $$QT_LIBS_PATH/Qt5PrintSupport$${QT_D}.dll \
                        $$QT_LIBS_PATH/Qt5MultimediaQuick$${QT_D}.dll
    }

    # Qt library dependencies
    qt5deps.path  = $$INSTALLROOT/$$LIBSDIR

    qt5deps.files += $$SYS_LIBS_PATH/libbz2-1.dll \
                     $$SYS_LIBS_PATH/libfreetype-6.dll \
                     $$SYS_LIBS_PATH/libglib-2.0-0.dll \
                     $$SYS_LIBS_PATH/libgraphite2.dll \
                     $$SYS_LIBS_PATH/libharfbuzz-0.dll \
                     $$SYS_LIBS_PATH/libiconv-2.dll \
                     $$SYS_LIBS_PATH/libintl-8.dll \
                     $$SYS_LIBS_PATH/libpcre-1.dll \
                     $$SYS_LIBS_PATH/libpng16-16.dll \
                     $$SYS_LIBS_PATH/libjpeg-8.dll \
                     $$SYS_LIBS_PATH/libspeex-1.dll \
                     $$SYS_LIBS_PATH/zlib1.dll

    qmlui: {
        qt5deps.files += $$SYS_LIBS_PATH/libassimp.dll \
                         $$SYS_LIBS_PATH/libminizip-1.dll
    }
    INSTALLS += qt5deps
}

INSTALLS += qtlibs

greaterThan(QT_MAJOR_VERSION, 4) {
    qtplatform.path = $$INSTALLROOT/$$LIBSDIR/platforms
    qtplatform.files = $$QT_PLUGINS_PATH/platforms/qwindows$${QT_D}.dll
    INSTALLS += qtplatform

    qtaudio.path = $$INSTALLROOT/$$LIBSDIR/audio
    qtaudio.files = $$QT_PLUGINS_PATH/audio/qtaudio_windows$${QT_D}.dll
    INSTALLS += qtaudio

    qtmedia.path = $$INSTALLROOT/$$LIBSDIR/mediaservice
    qtmedia.files = $$QT_PLUGINS_PATH/mediaservice/dsengine$${QT_D}.dll \
                    $$QT_PLUGINS_PATH/mediaservice/qtmedia_audioengine$${QT_D}.dll
    INSTALLS += qtmedia

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
}

# MSYS2 libraries
msys.path = $$INSTALLROOT/$$LIBSDIR

exists($$SYS_LIBS_PATH/libstdc++-6.dll) {
    msys.files += $$SYS_LIBS_PATH/libstdc++-6.dll
}

exists($$SYS_LIBS_PATH/libgcc_s_dw2-1.dll) {
    msys.files += $$SYS_LIBS_PATH/libgcc_s_dw2-1.dll
}

exists($$SYS_LIBS_PATH/libwinpthread-1.dll) {
    msys.files += $$SYS_LIBS_PATH/libwinpthread-1.dll
}

INSTALLS += msys

# audio libraries
audio.path = $$INSTALLROOT/$$LIBSDIR
exists($$SYS_LIBS_PATH/libmad-0.dll) {
    audio.files += $$SYS_LIBS_PATH/libmad-0.dll
}
exists($$SYS_LIBS_PATH/libogg-0.dll) {
    audio.files += $$SYS_LIBS_PATH/libogg-0.dll
}
exists($$SYS_LIBS_PATH/libvorbis-0.dll) {
    audio.files += $$SYS_LIBS_PATH/libvorbis-0.dll
}
exists($$SYS_LIBS_PATH/libvorbisenc-2.dll) {
    audio.files += $$SYS_LIBS_PATH/libvorbisenc-2.dll
}
exists($$SYS_LIBS_PATH/libFLAC-8.dll) {
    audio.files += $$SYS_LIBS_PATH/libFLAC-8.dll
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

lessThan(QT_MAJOR_VERSION, 5) {
    nsis.files = qlcplus4Qt4.nsi
} else {
    qmlui: {
        nsis.files = qlcplus5Qt5.nsi
    } else {
        nsis.files = qlcplus4Qt5.nsi
    }
}

INSTALLS  += nsis

samples.files += ../Sample.qxw
samples.path   = $$INSTALLROOT/$$DATADIR
INSTALLS      += samples
