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
include(libsndfile-nametool.pri)

contains(LIBS, -lqlcplusengine) {
    !isEmpty(nametool.commands) {
        nametool.commands += "&&"
    }

    nametool.commands += install_name_tool -change libqlcplusengine.1.dylib \
            @executable_path/../$$LIBSDIR/libqlcplusengine.1.dylib $$OUTFILE
}

# The contents of nametool.path don't matter; it only needs to be non-empty
nametool.path = $$INSTALLROOT
INSTALLS     += nametool
