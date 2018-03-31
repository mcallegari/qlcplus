include(../../variables.pri)

TEMPLATE = subdirs

QT_LIBS_PATH = $$dirname(QMAKE_QMAKE)
QT_PLUGINS_PATH = $$QT_LIBS_PATH/../share/qt5/plugins
QT_QML_PATH = $$QT_LIBS_PATH/../share/qt5/qml
SYS_LIBS_PATH = $$(SystemDrive)/msys64/mingw32/bin
#SYS_LIBS_PATH = D:/msys64/mingw32/bin

# Qt Libraries
qtlibs.path  = $$INSTALLROOT/$$LIBSDIR

lessThan(QT_MAJOR_VERSION, 5) {
    release:qtlibs.files = $$QT_LIBS_PATH/QtCore4.dll \
                           $$QT_LIBS_PATH/QtGui4.dll \
                           $$QT_LIBS_PATH/QtScript4.dll \
                           $$QT_LIBS_PATH/QtNetwork4.dll

    debug:qtlibs.files = $$QT_LIBS_PATH/QtCored4.dll \
                         $$QT_LIBS_PATH/QtGuid4.dll \
                         $$QT_LIBS_PATH/QtScriptd4.dll \
                         $$QT_LIBS_PATH/QtNetwork4d.dll
} else {
    release:qtlibs.files = $$QT_LIBS_PATH/Qt5Core.dll \
                           $$QT_LIBS_PATH/Qt5Script.dll \
                           $$QT_LIBS_PATH/Qt5Network.dll \
                           $$QT_LIBS_PATH/Qt5Gui.dll \
                           $$QT_LIBS_PATH/Qt5Widgets.dll \
                           $$QT_LIBS_PATH/Qt5OpenGL.dll \
                           $$QT_LIBS_PATH/Qt5Multimedia.dll \
                           $$QT_LIBS_PATH/Qt5MultimediaWidgets.dll

    debug:qtlibs.files = $$QT_LIBS_PATH/Qt5Cored.dll \
                         $$QT_LIBS_PATH/Qt5Scriptd.dll \
                         $$QT_LIBS_PATH/Qt5Networkd.dll \
                         $$QT_LIBS_PATH/Qt5Guid.dll \
                         $$QT_LIBS_PATH/Qt5Widgetsd.dll \
                         $$QT_LIBS_PATH/Qt5OpenGLd.dll \
                         $$QT_LIBS_PATH/Qt5Multimediad.dll \
                         $$QT_LIBS_PATH/Qt5MultimediaWidgetsd.dll

    qmlui: {
        release:qtlibs.files += $$QT_LIBS_PATH/Qt5Qml.dll \
                                $$QT_LIBS_PATH/Qt5Quick.dll \
                                $$QT_LIBS_PATH/Qt5QuickControls2.dll \
                                $$QT_LIBS_PATH/Qt5QuickTemplates2.dll \
                                $$QT_LIBS_PATH/Qt5Svg.dll \
                                $$QT_LIBS_PATH/Qt53DCore.dll \
                                $$QT_LIBS_PATH/Qt53DExtras.dll \
                                $$QT_LIBS_PATH/Qt53DInput.dll \
                                $$QT_LIBS_PATH/Qt53DLogic.dll \
                                $$QT_LIBS_PATH/Qt53DAnimation.dll \
                                $$QT_LIBS_PATH/Qt53DQuick.dll \
                                $$QT_LIBS_PATH/Qt53DQuickExtras.dll \
                                $$QT_LIBS_PATH/Qt53DQuickInput.dll \
                                $$QT_LIBS_PATH/Qt53DQuickRender.dll \
                                $$QT_LIBS_PATH/Qt53DRender.dll \
                                $$QT_LIBS_PATH/Qt5Concurrent.dll \
                                $$QT_LIBS_PATH/Qt5Gamepad.dll \
                                $$QT_LIBS_PATH/Qt5PrintSupport.dll
        lessThan(QT_MINOR_VERSION, 10) {
            release: qtlibs.files += $$QT_LIBS_PATH/Qt5MultimediaQuick_p.dll
        } else {
            release: qtlibs.files += $$QT_LIBS_PATH/Qt5MultimediaQuick.dll
        }
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
    debug:qtplatform.files = $$QT_PLUGINS_PATH/platforms/qwindowsd.dll
    release:qtplatform.files = $$QT_PLUGINS_PATH/platforms/qwindows.dll
    INSTALLS += qtplatform

    qtaudio.path = $$INSTALLROOT/$$LIBSDIR/audio
    debug:qtaudio.files = $$QT_PLUGINS_PATH/audio/qtaudio_windowsd.dll
    release:qtaudio.files = $$QT_PLUGINS_PATH/audio/qtaudio_windows.dll
    INSTALLS += qtaudio
    
    qtmedia.path = $$INSTALLROOT/$$LIBSDIR/mediaservice
    debug:qtmedia.files = $$QT_PLUGINS_PATH/mediaservice/dsengined.dll \
                          $$QT_PLUGINS_PATH/mediaservice/qtmedia_audioengined.dll
    release:qtmedia.files = $$QT_PLUGINS_PATH/mediaservice/dsengine.dll \
                            $$QT_PLUGINS_PATH/mediaservice/qtmedia_audioengine.dll
    INSTALLS += qtmedia

    qmlui: {
        qtimageformats.path = $$INSTALLROOT/$$LIBSDIR/imageformats
        qtimageformats.files = $$QT_PLUGINS_PATH/imageformats/qsvg.dll
        INSTALLS += qtimageformats

        qtprintsupport.path = $$INSTALLROOT/$$LIBSDIR/printsupport
        qtprintsupport.files = $$QT_PLUGINS_PATH/printsupport/windowsprintersupport.dll
        INSTALLS += qtprintsupport

        geometryloaders.path = $$INSTALLROOT/$$LIBSDIR/geometryloaders
        geometryloaders.files = $$QT_PLUGINS_PATH/geometryloaders/defaultgeometryloader.dll
        INSTALLS += geometryloaders

        sceneparsers.path = $$INSTALLROOT/$$LIBSDIR/sceneparsers
        sceneparsers.files = $$QT_PLUGINS_PATH/sceneparsers/assimpsceneimport.dll
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
                                  rm -rf Qt/WebSockets QtQuick/Extras QtQuick/Particles.2 QtQuick/XmlListModel \
                                  rm -rf QtQuick/Controls.2/designer QtQuick/Controls.2/Material \
                                  rm -rf QtQuick/Controls.2/Universal QtQuick/Controls.2/Fusion \
                                  rm -rf QtQuick/Controls.2/Imagine QtQuick/Controls.2/Scene2D
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
