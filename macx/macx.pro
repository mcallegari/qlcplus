include(../variables.pri)

TEMPLATE = subdirs
CONFIG  += ordered

include(libusb-nametool.pri)
include(libftdi-nametool.pri)
include(libmad-nametool.pri)
include(libsndfile-nametool.pri)
include(libportaudio-nametool.pri)
include(liblo-nametool.pri)
include(libfftw-nametool.pri)
include(libqtgui-nametool.pri)
include(libqtxml-nametool.pri)
include(libqtcore-nametool.pri)
include(libqtnetwork-nametool.pri)
include(libqtscript-nametool.pri)
include(libqlcplusengine-nametool.pri)
include(libqlcplusui-nametool.pri)
include(libqlcpluswebaccess-nametool.pri)

INSTALLS += LIBQLCENGINE_ID LIBQLCUI_ID LIBQLCWEBACCESS_ID
INSTALLS += LIBUSB LIBUSB_ID
INSTALLS += LIBFTDI LIBFTDI_ID
INSTALLS += LIBMAD LIBMAD_ID
INSTALLS += LIBSNDFILE LIBSNDFILE_ID
INSTALLS += LIBPORTAUDIO LIBPORTAUDIO_ID
INSTALLS += LIBLO LIBLO_ID
INSTALLS += LIBFFTW LIBFFTW_ID
INSTALLS += LIBQTGUI QTMENU LIBQTGUI_ID
INSTALLS += LIBQTXML LIBQTXML_ID
INSTALLS += LIBQTCORE LIBQTCORE_ID
INSTALLS += LIBQTNETWORK LIBQTNETWORK_ID
INSTALLS += LIBQTSCRIPT LIBQTSCRIPT_ID

# QtGui, QtXml, QtNetwork and QtScript depend on QtCore. Do this AFTER installing the
# libraries into the bundle
qtnametool.path = $$INSTALLROOT
qtnametool.commands = $$LIBQTCORE_INSTALL_NAME_TOOL \
    $$INSTALLROOT/$$LIBSDIR/$$LIBQTGUI_DIR/$$LIBQTGUI_FILE
qtnametool.commands += && $$LIBQTCORE_INSTALL_NAME_TOOL \
    $$INSTALLROOT/$$LIBSDIR/$$LIBQTXML_DIR/$$LIBQTXML_FILE
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
qtnametool.commands += && $$LIBPORTAUDIO_INSTALL_NAME_TOOL \
    $$INSTALLROOT/$$LIBSDIR/$$LIBQLCENGINE_FILE
qtnametool.commands += && $$LIBFFTW_INSTALL_NAME_TOOL \
    $$INSTALLROOT/$$LIBSDIR/$$LIBQLCENGINE_FILE

# libqlcplusui depends on libportaudio
qtnametool.commands += && $$LIBPORTAUDIO_INSTALL_NAME_TOOL \
    $$INSTALLROOT/$$LIBSDIR/$$LIBQLCUI_FILE

# libqlcpluswebaccess depends on libportaudio
qtnametool.commands += && $$LIBPORTAUDIO_INSTALL_NAME_TOOL \
    $$INSTALLROOT/$$LIBSDIR/$$LIBQLCWEBACCESS_FILE

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

INSTALLS += qtnametool
