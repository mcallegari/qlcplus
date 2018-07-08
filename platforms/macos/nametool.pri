#
# Including this file into a subproject .pro file will run install_name_tool
# for the subproject's $$TARGET to replace references to some libraries 
# with paths within the application bundle.

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

#############################################################
# $${1} : the pkg-config library name
# $${2} : the dylib library file name
#############################################################
defineReplace(pkgConfigNametool) {

    SYSLIB_DIR = $$system("pkg-config --variable libdir $${1}")

    return(install_name_tool -change $$SYSLIB_DIR/$$2 \
            @executable_path/../$$LIBSDIR/$$2 $$OUTFILE)
}

contains(LIBS, -lqlcplusengine) {
    !isEmpty(nametool.commands) {
        nametool.commands += "&&"
    }

    nametool.commands += install_name_tool -change libqlcplusengine.1.dylib \
            @executable_path/../$$LIBSDIR/libqlcplusengine.1.dylib $$OUTFILE
}

contains(LIBS, -lqlcplusui) {
    !isEmpty(nametool.commands) {
        nametool.commands += "&&"
    }

    nametool.commands += install_name_tool -change libqlcplusui.1.dylib \
            @executable_path/../$$LIBSDIR/libqlcplusui.1.dylib $$OUTFILE
}

contains(LIBS, -lqlcpluswebaccess) {
    !isEmpty(nametool.commands) {
        nametool.commands += "&&"
    }

    nametool.commands += install_name_tool -change libqlcpluswebaccess.1.dylib \
            @executable_path/../$$LIBSDIR/libqlcpluswebaccess.1.dylib $$OUTFILE
}

# The contents of nametool.path don't matter; it only needs to be non-empty
nametool.path = $$INSTALLROOT
INSTALLS     += nametool
