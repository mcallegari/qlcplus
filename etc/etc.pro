include(../variables.pri)

TEMPLATE = subdirs
TARGET   = icons

# Linux
unix:!macx {
    desktop.path   = $$INSTALLROOT/share/applications/
    desktop.files += qlc.desktop qlc-fixtureeditor.desktop
    INSTALLS      += desktop

    icons.path   = $$INSTALLROOT/share/pixmaps/
    icons.files += ../gfx/qlc.png ../gfx/qlc-fixtureeditor.png
    INSTALLS    += icons

    mime.path   = $$INSTALLROOT/share/mime/packages
    mime.files += qlc.xml
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
    icons.files += ../gfx/qlc.icns

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

    # NullSoft installer files
    nsis.path  = $$INSTALLROOT/$$DATADIR
    nsis.files = qlc.nsi
    INSTALLS  += nsis
}

samples.files += Sample.qxw
samples.path   = $$INSTALLROOT/$$DATADIR
INSTALLS      += samples
