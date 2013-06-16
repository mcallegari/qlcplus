include(../variables.pri)

TEMPLATE = subdirs
TARGET   = icons

# Linux
unix:!macx {
    desktop.path   = $$INSTALLROOT/share/applications/
    desktop.files += qlcplus.desktop qlcplus-fixtureeditor.desktop
    INSTALLS      += desktop

    icons.path   = $$INSTALLROOT/share/pixmaps/
    icons.files += ../gfx/qlcplus.png ../gfx/qlcplus-fixtureeditor.png
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
    icons.files += ../gfx/qlcplus.icns

    plist.path   = $$INSTALLROOT
    plist.files += Info.plist
    INSTALLS    += icons plist
}

# Windows
win32 {
    # Qt Libraries
    qtlibs.path  = $$INSTALLROOT/$$LIBSDIR
    release:qtlibs.files = $$(QTDIR)/bin/QtCore4.dll \
                           $$(QTDIR)/bin/QtGui4.dll \
                           $$(QTDIR)/bin/QtXml4.dll \
                           $$(QTDIR)/bin/QtScript4.dll

    debug:qtlibs.files = $$(QTDIR)/bin/QtCored4.dll \
                         $$(QTDIR)/bin/QtGuid4.dll \
                         $$(QTDIR)/bin/QtXmld4.dll \
                         $$(QTDIR)/bin/QtScriptd4.dll
    INSTALLS += qtlibs

    # MinGW library
    mingw.path = $$INSTALLROOT/$$LIBSDIR
    exists($$(SystemDrive)/MinGW/bin/mingwm10.dll) {
        mingw.files += $$(SystemDrive)/MinGW/bin/mingwm10.dll
    }

    exists($$(QTDIR)/../MinGW/bin/mingwm10.dll) {
        mingw.files += $$(QTDIR)/../MinGW/bin/mingwm10.dll
    }

    # GCC 4.4.0
    exists($$(SystemDrive)/MinGW/bin/libgcc_s_dw2-1.dll) {
        mingw.files += $$(SystemDrive)/MinGW/bin/libgcc_s_dw2-1.dll
    }

    exists($$(QTDIR)/../MinGW/bin/libgcc_s_dw2-1.dll) {
        mingw.files += $$(QTDIR)/../MinGW/bin/libgcc_s_dw2-1.dll
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
    nsis.files = qlcplus.nsi
    INSTALLS  += nsis
}

samples.files += Sample.qxw
samples.path   = $$INSTALLROOT/$$DATADIR
INSTALLS      += samples
