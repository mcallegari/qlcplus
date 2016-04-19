include(../variables.pri)

TEMPLATE = subdirs
TARGET   = icons

# Linux
unix:!macx {
    desktop.path   = $$INSTALLROOT/share/applications/
    desktop.files += qlcplus.desktop qlcplus-fixtureeditor.desktop
    INSTALLS      += desktop

    icons.path   = $$INSTALLROOT/share/pixmaps/
    icons.files += ../resources/icons/png/qlcplus.png ../resources/icons/png/qlcplus-fixtureeditor.png
    INSTALLS    += icons

    mime.path   = $$INSTALLROOT/share/mime/packages
    mime.files += qlcplus.xml
    INSTALLS   += mime

    appdata.path   = $$INSTALLROOT/share/appdata/
    appdata.files += qlcplus-fixtureeditor.appdata.xml qlcplus.appdata.xml
    INSTALLS      += appdata

    manpages.path = $$INSTALLROOT/$$MANDIR
    manpages.files += *.1
    INSTALLS += manpages

    # This is nowadays run by dpkg (TODO: rpm)
    #MIMEUPDATE    = $$system("which update-mime-database")
    #mimeupdate.commands = $$MIMEUPDATE /usr/share/mime
    #mimeupdate.path = /usr/share/mime
    #INSTALLS += mimeupdate
}

# Mac OSX
macx {
    icons.path   = $$INSTALLROOT/$$DATADIR
    icons.files += ../resources/icons/qlcplus.icns

    plist.path   = $$INSTALLROOT
    plist.files += Info.plist
    INSTALLS    += icons plist
}

# Windows
win32 {
    QT_LIBS_PATH = $$dirname(QMAKE_QMAKE)
    QT_PLUGINS_PATH = $$QT_LIBS_PATH/../share/qt5/plugins
    SYS_LIBS_PATH = $$(SystemDrive)/msys64/mingw32/bin

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
                           $$QT_LIBS_PATH/Qt5Widgets.dll \
                           $$QT_LIBS_PATH/Qt5OpenGL.dll \
                           $$QT_LIBS_PATH/Qt5Multimedia.dll \
                           $$QT_LIBS_PATH/Qt5MultimediaWidgets.dll

    debug:qtlibs.files = $$QT_LIBS_PATH/Qt5Cored.dll \
                         $$QT_LIBS_PATH/Qt5Scriptd.dll \
                         $$QT_LIBS_PATH/Qt5Networkd.dll \
                         $$QT_LIBS_PATH/Qt5Widgetsd.dll \
                         $$QT_LIBS_PATH/Qt5OpenGLd.dll \
                         $$QT_LIBS_PATH/Qt5Multimediad.dll \
                         $$QT_LIBS_PATH/Qt5MultimediaWidgetsd.dll

    qmlui: {
        release:qtlibs.files += $$QT_LIBS_PATH/Qt5Qml.dll \
                                $$QT_LIBS_PATH/Qt5Quick.dll \
                                $$QT_LIBS_PATH/Qt5Svg.dll
    } else {
        release:qtlibs.files += $$QT_LIBS_PATH/Qt5Gui.dll
        debug:qtlibs.files += $$QT_LIBS_PATH/Qt5Guid.dll
    }

    # Qt Libraries
    qt5deps.path  = $$INSTALLROOT/$$LIBSDIR

    qt5deps.files += \
                     $$SYS_LIBS_PATH/libbz2-1.dll \
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
        debug:qtimageformats.files = $$QT_PLUGINS_PATH/imageformats/qsvgd.dll
        release:qtimageformats.files = $$QT_PLUGINS_PATH/imageformats/qsvg.dll
        INSTALLS += qtimageformats
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
    nsis.files = qlcplus.nsi
} else {
    nsis.files = qlcplusQt5.nsi
}
    INSTALLS  += nsis
}

samples.files += Sample.qxw
samples.path   = $$INSTALLROOT/$$DATADIR
INSTALLS      += samples
