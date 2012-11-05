#
# Including this file into a subproject .pro file will run install_name_tool
# for the subproject's $$TARGET to replace references to QtCore, QtGui, QtXml,
# QtNetwork, libusb and libftdi with paths within the application bundle.
#

# Libraries
contains(TEMPLATE, lib) {
    OUTFILE = libTARGET.dylib
    OUTFILE = $$replace(OUTFILE, TARGET, $$TARGET)
}

# Executables
contains(TEMPLATE, app) {
    OUTFILE = $$TARGET
}

include(libusb-nametool.pri)
include(libftdi-nametool.pri)
include(libqtgui-nametool.pri)
include(libqtxml-nametool.pri)
include(libqtcore-nametool.pri)
include(libqtnetwork-nametool.pri)
include(libqtscript-nametool.pri)
include(imageformats-nametool.pri)
include(libqlcengine-nametool.pri)
include(libqlcui-nametool.pri)

# The contents of nametool.path don't matter; it only needs to be non-empty
nametool.path = $$INSTALLROOT
INSTALLS     += nametool
