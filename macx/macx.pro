include(../variables.pri)

TEMPLATE = subdirs
CONFIG  += ordered

include(libusb-nametool.pri)
include(libftdi-nametool.pri)
include(libqtgui-nametool.pri)
include(libqtxml-nametool.pri)
include(libqtcore-nametool.pri)
include(libqtnetwork-nametool.pri)
include(libqtscript-nametool.pri)
include(libqlcengine-nametool.pri)
include(libqlcui-nametool.pri)

INSTALLS += LIBQLCENGINE_ID LIBQLCUI_ID
INSTALLS += LIBUSB LIBUSB_ID
INSTALLS += LIBFTDI LIBFTDI_ID
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

# These never had any difference anyway...
# include(imageformats-nametool.pri)
# INSTALLS += imageformats

INSTALLS += qtnametool
