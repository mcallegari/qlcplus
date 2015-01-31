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
    # Qt Libraries
    qtlibs.path  = $$INSTALLROOT/$$LIBSDIR

lessThan(QT_MAJOR_VERSION, 5) {
    release:qtlibs.files = $$(QTDIR)/bin/QtCore4.dll \
                           $$(QTDIR)/bin/QtGui4.dll \
                           $$(QTDIR)/bin/QtXml4.dll \
                           $$(QTDIR)/bin/QtScript4.dll \
                           $$(QTDIR)/bin/QtNetwork4.dll

    debug:qtlibs.files = $$(QTDIR)/bin/QtCored4.dll \
                         $$(QTDIR)/bin/QtGuid4.dll \
                         $$(QTDIR)/bin/QtXmld4.dll \
                         $$(QTDIR)/bin/QtScriptd4.dll \
                         $$(QTDIR)/bin/QtNetwork4d.dll
} else {
    release:qtlibs.files = $$(QTDIR)/bin/Qt5Core.dll \
                           $$(QTDIR)/bin/Qt5Gui.dll \
                           $$(QTDIR)/bin/Qt5Xml.dll \
                           $$(QTDIR)/bin/Qt5Script.dll \
                           $$(QTDIR)/bin/Qt5Network.dll \
                           $$(QTDIR)/bin/Qt5Widgets.dll \
                           $$(QTDIR)/bin/Qt5OpenGL.dll \
                           $$(QTDIR)/bin/Qt5Multimedia.dll \
                           $$(QTDIR)/bin/Qt5MultimediaWidgets.dll

    debug:qtlibs.files = $$(QTDIR)/bin/Qt5Cored.dll \
                         $$(QTDIR)/bin/Qt5Guid.dll \
                         $$(QTDIR)/bin/Qt5Xmld.dll \
                         $$(QTDIR)/bin/Qt5Scriptd.dll \
                         $$(QTDIR)/bin/Qt5Networkd.dll \
                         $$(QTDIR)/bin/Qt5Widgetsd.dll \
                         $$(QTDIR)/bin/Qt5OpenGLd.dll \
                         $$(QTDIR)/bin/Qt5Multimediad.dll \
                         $$(QTDIR)/bin/Qt5MultimediaWidgetsd.dll 
    qtlibs.files += $$(QTDIR)/bin/icudt52.dll \
                    $$(QTDIR)/bin/icuin52.dll \
                    $$(QTDIR)/bin/icuuc52.dll
}
    INSTALLS += qtlibs

greaterThan(QT_MAJOR_VERSION, 4) {
    qtplatform.path = $$INSTALLROOT/$$LIBSDIR/platforms
    debug:qtplatform.files = $$(QTDIR)/plugins/platforms/qwindowsd.dll
    release:qtplatform.files = $$(QTDIR)/plugins/platforms/qwindows.dll
    INSTALLS += qtplatform
	
	qtaudio.path = $$INSTALLROOT/$$LIBSDIR/audio
    debug:qtaudio.files = $$(QTDIR)/plugins/audio/qtaudio_windowsd.dll
    release:qtaudio.files = $$(QTDIR)/plugins/audio/qtaudio_windows.dll
    INSTALLS += qtaudio
    
    qtmedia.path = $$INSTALLROOT/$$LIBSDIR/mediaservice
    debug:qtmedia.files = $$(QTDIR)/plugins/mediaservice/dsengined.dll \
                          $$(QTDIR)/plugins/mediaservice/qtmedia_audioengined.dll
    release:qtmedia.files = $$(QTDIR)/plugins/mediaservice/dsengine.dll \
                            $$(QTDIR)/plugins/mediaservice/qtmedia_audioengine.dll
    INSTALLS += qtmedia
}

    # MinGW library
    mingw.path = $$INSTALLROOT/$$LIBSDIR
    exists($$(SystemDrive)/MinGW/bin/mingwm10.dll) {
        mingw.files += $$(SystemDrive)/MinGW/bin/mingwm10.dll
    }

    # MinGW GCC
    exists($$(SystemDrive)/MinGW/bin/libgcc_s_dw2-1.dll) {
        mingw.files += $$(SystemDrive)/MinGW/bin/libgcc_s_dw2-1.dll
    }

lessThan(QT_MAJOR_VERSION, 5) {
    exists($$(SystemDrive)/MinGW/bin/libstdc++-6.dll) {
        mingw.files += $$(SystemDrive)/MinGW/bin/libstdc++-6.dll
    }
} else {
    exists($$(QTDIR)/bin/libstdc++-6.dll) {
        mingw.files += $$(QTDIR)/bin/libstdc++-6.dll
    }
}

    exists($$(SystemDrive)/MinGW/bin/pthreadGC2.dll) {
        mingw.files += $$(SystemDrive)/MinGW/bin/pthreadGC2.dll
    }
    exists($$(QTDIR)/bin/libwinpthread-1.dll) {
        mingw.files += $$(QTDIR)/bin/libwinpthread-1.dll
    }

    INSTALLS += mingw

        # audio libraries
        audio.path = $$INSTALLROOT/$$LIBSDIR
        exists($$(SystemDrive)/MinGW/bin/libmad-0.dll) {
        audio.files += $$(SystemDrive)/MinGW/bin/libmad-0.dll
    }
        exists($$(SystemDrive)/MinGW/bin/libogg-0.dll) {
        audio.files += $$(SystemDrive)/MinGW/bin/libogg-0.dll
    }
        exists($$(SystemDrive)/MinGW/bin/libvorbis-0.dll) {
        audio.files += $$(SystemDrive)/MinGW/bin/libvorbis-0.dll
    }
        exists($$(SystemDrive)/MinGW/bin/libvorbisenc-2.dll) {
        audio.files += $$(SystemDrive)/MinGW/bin/libvorbisenc-2.dll
    }
        exists($$(SystemDrive)/MinGW/bin/libFLAC-8.dll) {
        audio.files += $$(SystemDrive)/MinGW/bin/libFLAC-8.dll
    }
        exists($$(SystemDrive)/MinGW/bin/libsndfile-1.dll) {
        audio.files += $$(SystemDrive)/MinGW/bin/libsndfile-1.dll
    }
        exists($$(SystemDrive)/MinGW/bin/libfftw3-3.dll) {
        audio.files += $$(SystemDrive)/MinGW/bin/libfftw3-3.dll
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
