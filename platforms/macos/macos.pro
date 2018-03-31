include(../../variables.pri)

TEMPLATE = subdirs
CONFIG  += ordered
TARGET   = icons

#############################################################
# $${1} : the target label
# $${2} : the dylib library file name
#############################################################
defineReplace(libraryTargetID) {
    # export library file
    eval($${1}_FILE = $${2})
    eval(export($${1}_FILE))

    # export the install target ID
    eval($${1}_ID.path = $$INSTALLROOT/$$LIBSDIR)
    eval($${1}_ID.commands = install_name_tool -id @executable_path/../$$LIBSDIR/$${2} $$INSTALLROOT/$$LIBSDIR/$${2})
    eval(export($${1}_ID.path))
    eval(export($${1}_ID.commands))

    return($${1}_ID)
}

#############################################################
# $${1} : the target label
# $${2} : the dylib library file name
# $${3} : the pkgconfig library name
#############################################################
defineReplace(systemLibTarget) {
    # export library file
    eval($${1}_FILE = $${2})
    eval(export($${1}_FILE))

    SYSLIB_DIR = $$system("pkg-config --variable libdir $${3}")

    # export the nametool variable
    eval($${1}_INSTALL_NAME_TOOL = install_name_tool -change $$SYSLIB_DIR/$${2} @executable_path/../$$LIBSDIR/$${2})
    eval(export($${1}_INSTALL_NAME_TOOL))

    # export the library install target
    eval($${1}.path = $$INSTALLROOT/$$LIBSDIR)
    eval($${1}.files = $$SYSLIB_DIR/$${2})
    eval(export($${1}.path))
    eval(export($${1}.files))

    return($${1})
}

#############################################################
# $${1} : the target label
# $${2} : the Qt framework basename
#############################################################
defineReplace(qt5LibTarget) {
    # export framework dir
    QTFRAMEWORK_DIR = $${2}.framework/Versions/5
    eval($${1}_DIR = $$QTFRAMEWORK_DIR)
    eval(export($${1}_DIR))

    # export the nametool variable
    eval($${1}_INSTALL_NAME_TOOL = install_name_tool -change $$(QTDIR)/lib/$$QTFRAMEWORK_DIR/$${2} @executable_path/../$$LIBSDIR/$$QTFRAMEWORK_DIR/$${2})
    eval(export($${1}_INSTALL_NAME_TOOL))

    # export the library install target
    eval($${1}.path = $$INSTALLROOT/$$LIBSDIR/$$QTFRAMEWORK_DIR)
    eval($${1}.files = $$(QTDIR)/lib/$$QTFRAMEWORK_DIR/$${2})
    eval(export($${1}.path))
    eval(export($${1}.files))

    return($${1})
}

#############################################################
# $${1} : the target label
# $${2} : the Qt framework basename
#############################################################
defineReplace(qt5LibTargetID) {
    QTFRAMEWORK_DIR = $${2}.framework/Versions/5

    # export library file
    eval($${1}_FILE = $${2})
    eval(export($${1}_FILE))

    # export the install target ID
    eval($${1}_ID.path = $$INSTALLROOT/$$LIBSDIR/$$QTFRAMEWORK_DIR)
    eval($${1}_ID.commands = install_name_tool -id @executable_path/../$$LIBSDIR/$$QTFRAMEWORK_DIR/$${2} $$INSTALLROOT/$$LIBSDIR/$$QTFRAMEWORK_DIR/$${2})
    eval(export($${1}_ID.path))
    eval(export($${1}_ID.commands))

    return($${1}_ID)
}

include(libusb-nametool.pri)
include(libsndfile-nametool.pri)

!qmlui: {
 INSTALLS += $$libraryTargetID(LIBQLCUI, libqlcplusui.1.dylib)
 INSTALLS += $$libraryTargetID(LIBQLCWEBACCESS, libqlcpluswebaccess.1.dylib)
}

INSTALLS += $$libraryTargetID(LIBQLCENGINE, libqlcplusengine.1.dylib)
INSTALLS += LIBUSB LIBUSB_ID
INSTALLS += $$systemLibTarget(LIBFTDI, libftdi.1.dylib, libftdi) $$libraryTargetID(LIBFTDI, libftdi.1.dylib)
INSTALLS += $$systemLibTarget(LIBMAD, libmad.0.dylib, mad) $$libraryTargetID(LIBMAD, libmad.0.dylib)
INSTALLS += LIBSNDFILE LIBSNDFILE_ID
INSTALLS += $$systemLibTarget(LIBFFTW, libfftw3.3.dylib, fftw3) $$libraryTargetID(LIBFFTW, libfftw3.3.dylib)

INSTALLS += $$qt5LibTarget(LIBQTCORE, QtCore) $$qt5LibTargetID(LIBQTCORE, QtCore)
INSTALLS += $$qt5LibTarget(LIBQTGUI, QtGui) 
QTMENU.files += $$(QTDIR)/lib/$$LIBQTGUI_DIR/Resources/*
QTMENU.path = $$INSTALLROOT/$$DATADIR
INSTALLS += QTMENU
INSTALLS += $$qt5LibTargetID(LIBQTGUI, QtGui)
INSTALLS += $$qt5LibTarget(LIBQTNETWORK, QtNetwork) $$qt5LibTargetID(LIBQTNETWORK, QtNetwork)
INSTALLS += $$qt5LibTarget(LIBQTSCRIPT, QtScript) $$qt5LibTargetID(LIBQTSCRIPT, QtScript)

greaterThan(QT_MAJOR_VERSION, 4) {
  INSTALLS += $$qt5LibTarget(LIBQTWIDGETS, QtWidgets) $$qt5LibTargetID(LIBQTWIDGETS, QtWidgets)
  INSTALLS += $$qt5LibTarget(LIBQTOPENGL, QtOpenGL) $$qt5LibTargetID(LIBQTOPENGL, QtOpenGL)
  INSTALLS += $$qt5LibTarget(LIBQTMULTIMEDIA, QtMultimedia) $$qt5LibTargetID(LIBQTMULTIMEDIA, QtMultimedia)
  INSTALLS += $$qt5LibTarget(LIBQTMULTIMEDIAWIDGETS, QtMultimediaWidgets) $$qt5LibTargetID(LIBQTMULTIMEDIAWIDGETS, QtMultimediaWidgets)
  INSTALLS += $$qt5LibTarget(LIBQTPRINTSUPPORT, QtPrintSupport) $$qt5LibTargetID(LIBQTPRINTSUPPORT, QtPrintSupport)
  INSTALLS += $$qt5LibTarget(LIBQTSERIALPORT, QtSerialPort) $$qt5LibTargetID(LIBQTSERIALPORT, QtSerialPort)
  greaterThan(QT_MINOR_VERSION, 4) {
    INSTALLS += $$qt5LibTarget(LIBQTDBUS, QtDBus) $$qt5LibTargetID(LIBQTDBUS, QtDBus)
  }
}

qmlui: {
  INSTALLS += $$qt5LibTarget(LIBQTQML, QtQml) $$qt5LibTargetID(LIBQTQML, QtQml)
  INSTALLS += $$qt5LibTarget(LIBQTQUICK, QtQuick) $$qt5LibTargetID(LIBQTQUICK, QtQuick)
  INSTALLS += $$qt5LibTarget(LIBQTQUICKCONTROLS2, QtQuickControls2) $$qt5LibTargetID(LIBQTQUICKCONTROLS2, QtQuickControls2)
  INSTALLS += $$qt5LibTarget(LIBQTQUICKTEMPLATES2, QtQuickTemplates2) $$qt5LibTargetID(LIBQTQUICKTEMPLATES2, QtQuickTemplates2)
  INSTALLS += $$qt5LibTarget(LIBQTSVG, QtSvg) $$qt5LibTargetID(LIBQTSVG, QtSvg)
  INSTALLS += $$qt5LibTarget(LIBQTCONCURRENT, QtConcurrent) $$qt5LibTargetID(LIBQTCONCURRENT, QtConcurrent)
  INSTALLS += $$qt5LibTarget(LIBQTGAMEPAD, QtGamepad) $$qt5LibTargetID(LIBQTGAMEPAD, QtGamepad)
  INSTALLS += $$qt5LibTarget(LIBQT3DCORE, Qt3DCore) $$qt5LibTargetID(LIBQT3DCORE, Qt3DCore)
  INSTALLS += $$qt5LibTarget(LIBQT3DRENDER, Qt3DRender) $$qt5LibTargetID(LIBQT3DRENDER, Qt3DRender)
  INSTALLS += $$qt5LibTarget(LIBQT3DEXTRAS, Qt3DExtras) $$qt5LibTargetID(LIBQT3DEXTRAS, Qt3DExtras)
  INSTALLS += $$qt5LibTarget(LIBQT3DINPUT, Qt3DInput) $$qt5LibTargetID(LIBQT3DINPUT, Qt3DInput)
  INSTALLS += $$qt5LibTarget(LIBQT3DLOGIC, Qt3DLogic) $$qt5LibTargetID(LIBQT3DLOGIC, Qt3DLogic)
  INSTALLS += $$qt5LibTarget(LIBQT3DANIMATION, Qt3DAnimation) $$qt5LibTargetID(LIBQT3DANIMATION, Qt3DAnimation)
  INSTALLS += $$qt5LibTarget(LIBQT3DQUICK, Qt3DQuick) $$qt5LibTargetID(LIBQT3DQUICK, Qt3DQuick)
  INSTALLS += $$qt5LibTarget(LIBQT3DQUICKRENDER, Qt3DQuickRender) $$qt5LibTargetID(LIBQT3DQUICKRENDER, Qt3DQuickRender)
  INSTALLS += $$qt5LibTarget(LIBQT3DQUICKINPUT, Qt3DQuickInput) $$qt5LibTargetID(LIBQT3DQUICKINPUT, Qt3DQuickInput)
  INSTALLS += $$qt5LibTarget(LIBQT3DQUICKEXTRAS, Qt3DQuickExtras) $$qt5LibTargetID(LIBQT3DQUICKEXTRAS, Qt3DQuickExtras)
  lessThan(QT_MINOR_VERSION, 10) {
    INSTALLS += $$qt5LibTarget(LIBQTMULTIMEDIAQUICK, QtMultimediaQuick_p) $$qt5LibTargetID(LIBQTMULTIMEDIAQUICK, QtMultimediaQuick_p)
  } else {
    INSTALLS += $$qt5LibTarget(LIBQTMULTIMEDIAQUICK, QtMultimediaQuick) $$qt5LibTargetID(LIBQTMULTIMEDIAQUICK, QtMultimediaQuick)
  }
}

# QtGui, QtNetwork and QtScript depend on QtCore.
# Do this AFTER installing the libraries into the bundle
qtnametool.path = $$INSTALLROOT

qtnametool.commands = $$LIBQTCORE_INSTALL_NAME_TOOL \
    $$INSTALLROOT/$$LIBSDIR/$$LIBQTGUI_DIR/$$LIBQTGUI_FILE
qtnametool.commands += && $$LIBQTCORE_INSTALL_NAME_TOOL \
    $$INSTALLROOT/$$LIBSDIR/$$LIBQTNETWORK_DIR/$$LIBQTNETWORK_FILE
qtnametool.commands += && $$LIBQTCORE_INSTALL_NAME_TOOL \
    $$INSTALLROOT/$$LIBSDIR/$$LIBQTSCRIPT_DIR/$$LIBQTSCRIPT_FILE

# Libftdi depends on libusb0.1 & 1.0
qtnametool.commands += && $$LIBUSB0_INSTALL_NAME_TOOL \
    $$INSTALLROOT/$$LIBSDIR/$$LIBFTDI_FILE
qtnametool.commands += && $$LIBUSB1_INSTALL_NAME_TOOL \
    $$INSTALLROOT/$$LIBSDIR/$$LIBFTDI_FILE

# Libusb0.1 depends on libusb1.0
qtnametool.commands += && $$LIBUSB1_INSTALL_NAME_TOOL \
    $$INSTALLROOT/$$LIBSDIR/$$LIBUSB0_FILE

# libqlcplusengine depends on libmad, libsndfile, libportaudio and libfftw3
qtnametool.commands += && $$LIBMAD_INSTALL_NAME_TOOL \
    $$INSTALLROOT/$$LIBSDIR/$$LIBQLCENGINE_FILE
qtnametool.commands += && $$LIBSNDFILE_INSTALL_NAME_TOOL \
    $$INSTALLROOT/$$LIBSDIR/$$LIBQLCENGINE_FILE

lessThan(QT_MAJOR_VERSION, 5) {
    # libqlcplusengine depends on libportaudio
    qtnametool.commands += && $$LIBPORTAUDIO_INSTALL_NAME_TOOL \
        $$INSTALLROOT/$$LIBSDIR/$$LIBQLCENGINE_FILE
    # libqlcplusui depends on libportaudio
    qtnametool.commands += && $$LIBPORTAUDIO_INSTALL_NAME_TOOL \
        $$INSTALLROOT/$$LIBSDIR/$$LIBQLCUI_FILE
    # libqlcpluswebaccess depends on libportaudio
    qtnametool.commands += && $$LIBPORTAUDIO_INSTALL_NAME_TOOL \
        $$INSTALLROOT/$$LIBSDIR/$$LIBQLCWEBACCESS_FILE
} else {
    qtnametool.commands += && $$LIBQTNETWORK_INSTALL_NAME_TOOL \
        $$INSTALLROOT/$$LIBSDIR/$$LIBQLCENGINE_FILE
!qmlui: {
    qtnametool.commands += && $$LIBQTNETWORK_INSTALL_NAME_TOOL \
        $$INSTALLROOT/$$LIBSDIR/$$LIBQLCUI_FILE
    qtnametool.commands += && $$LIBQTNETWORK_INSTALL_NAME_TOOL \
        $$INSTALLROOT/$$LIBSDIR/$$LIBQLCWEBACCESS_FILE
}
}

qtnametool.commands += && $$LIBFFTW_INSTALL_NAME_TOOL \
    $$INSTALLROOT/$$LIBSDIR/$$LIBQLCENGINE_FILE

# libsndfile depends on flac, libvorbis, libvorbisenc and libogg
qtnametool.commands += && $$LIBOGG_INSTALL_NAME_TOOL \
    $$INSTALLROOT/$$LIBSDIR/$$LIBSNDFILE_FILE
qtnametool.commands += && $$LIBFLAC_INSTALL_NAME_TOOL \
    $$INSTALLROOT/$$LIBSDIR/$$LIBSNDFILE_FILE
qtnametool.commands += && $$LIBVORBIS_INSTALL_NAME_TOOL \
    $$INSTALLROOT/$$LIBSDIR/$$LIBSNDFILE_FILE
qtnametool.commands += && $$LIBVORBISENC_INSTALL_NAME_TOOL \
    $$INSTALLROOT/$$LIBSDIR/$$LIBSNDFILE_FILE

# libFLAC depends on libogg
qtnametool.commands += && $$LIBOGG_INSTALL_NAME_TOOL \
    $$INSTALLROOT/$$LIBSDIR/$$LIBFLAC_FILE

# libvorbis depends on libogg
qtnametool.commands += && $$LIBOGG_INSTALL_NAME_TOOL \
    $$INSTALLROOT/$$LIBSDIR/$$LIBVORBIS_FILE

# libvorbisenc depends on libvorbis and libogg
qtnametool.commands += && $$LIBVORBIS_INSTALL_NAME_TOOL \
    $$INSTALLROOT/$$LIBSDIR/$$LIBVORBISENC_FILE
qtnametool.commands += && $$LIBOGG_INSTALL_NAME_TOOL \
    $$INSTALLROOT/$$LIBSDIR/$$LIBVORBISENC_FILE

# These never had any difference anyway...
# include(imageformats-nametool.pri)
# INSTALLS += imageformats

greaterThan(QT_MAJOR_VERSION, 4) {
    include(platformplugins-nametool.pri)
    include(audioplugins-nametool.pri)
    include(mediaservice-nametool.pri)

    INSTALLS += platformplugins
    INSTALLS += audioplugins
    INSTALLS += mediaservice

qmlui: {
    include(imageformats-nametool.pri)
    include(printsupport-nametool.pri)
    include(geometryloaders-nametool.pri)
    include(sceneparsers-nametool.pri)
    
    INSTALLS += imageformats
    INSTALLS += printsupport
    INSTALLS += geometryloaders
    INSTALLS += sceneparsers

# QML components
    qmlqtdeps.path   = $$INSTALLROOT/qml/Qt/labs
    qmlqtdeps.files += $$(QTDIR)/qml/Qt/labs/folderlistmodel \
                       $$(QTDIR)/qml/Qt/labs/settings
    INSTALLS += qmlqtdeps

    qmldeps.path   = $$INSTALLROOT/qml
    qmldeps.files += $$(QTDIR)/qml/Qt \
                     $$(QTDIR)/qml/QtQml \
                     $$(QTDIR)/qml/QtQuick \
                     $$(QTDIR)/qml/QtQuick.2 \
                     $$(QTDIR)/qml/Qt3D \
                     $$(QTDIR)/qml/QtMultimedia

    INSTALLS += qmldeps

    qmlpostinstall.path = $$INSTALLROOT/qml
    qmlpostinstall.commands = cd $$INSTALLROOT/qml && \
                              find . -name *_debug.dylib -type f -delete && \
                              find . -name plugins.qmltypes -type f -delete && \
                              find . -name *.qmlc -type f -delete && \
                              rm -rf Qt/WebSockets rm -rf QtQuick/Extras QtQuick/Particles.2 QtQuick/XmlListModel \
                              rm -rf QtQuick/Controls.2/designer QtQuick/Controls.2/Material \
                              rm -rf QtQuick/Controls.2/Universal QtQuick/Controls.2/Fusion \
                              rm -rf QtQuick/Controls.2/Imagine QtQuick/Controls.2/Scene2D
    INSTALLS  += qmlpostinstall
}

    qtconf.path   = $$INSTALLROOT/Resources
    qtconf.files += qt.conf
    INSTALLS      += qtconf
}

icons.path   = $$INSTALLROOT/$$DATADIR
icons.files += ../../resources/icons/qlcplus.icns

qmlui: {
    plist.path   = $$INSTALLROOT
    plist.commands = cp Info.plist.qmlui $$INSTALLROOT/Info.plist
} else {
    plist.path   = $$INSTALLROOT
    plist.files += Info.plist
}
INSTALLS    += icons plist

samples.files += ../Sample.qxw
samples.path   = $$INSTALLROOT/$$DATADIR
INSTALLS      += samples

INSTALLS += qtnametool
