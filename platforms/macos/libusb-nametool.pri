LIBUSB0_DIR      = $$system("pkg-config --variable libdir libusb")
LIBUSB0_FILE     = libusb-0.1.4.dylib
LIBUSB0_FILEPATH = $$LIBUSB0_DIR/$$LIBUSB0_FILE

LIBUSB1_DIR      = $$system("pkg-config --variable libdir libusb-1.0")
LIBUSB1_FILE     = libusb-1.0.0.dylib
LIBUSB1_FILEPATH = $$LIBUSB1_DIR/$$LIBUSB1_FILE

LIBUSB0_INSTALL_NAME_TOOL = install_name_tool -change $$LIBUSB0_FILEPATH \
                @executable_path/../$$LIBSDIR/$$LIBUSB0_FILE
LIBUSB1_INSTALL_NAME_TOOL = install_name_tool -change $$LIBUSB1_FILEPATH \
                @executable_path/../$$LIBSDIR/$$LIBUSB1_FILE

contains(PKGCONFIG, libusb) {
    !isEmpty(nametool.commands) {
        nametool.commands += "&&"
    }

    nametool.commands += $$LIBUSB0_INSTALL_NAME_TOOL $$OUTFILE && \
                 $$LIBUSB1_INSTALL_NAME_TOOL $$OUTFILE
}

LIBUSB.path   = $$INSTALLROOT/$$LIBSDIR
LIBUSB.files += $$LIBUSB0_FILEPATH
LIBUSB.files += $$LIBUSB1_FILEPATH

LIBUSB0_INSTALL_NAME_TOOL_ID = install_name_tool -id @executable_path/../$$LIBSDIR/$$LIBUSB0_FILE \
                $$INSTALLROOT/$$LIBSDIR/$$LIBUSB0_FILE
LIBUSB1_INSTALL_NAME_TOOL_ID = install_name_tool -id @executable_path/../$$LIBSDIR/$$LIBUSB1_FILE \
                $$INSTALLROOT/$$LIBSDIR/$$LIBUSB1_FILE
LIBUSB_ID.path     = $$INSTALLROOT/$$LIBSDIR
LIBUSB_ID.commands = $$LIBUSB0_INSTALL_NAME_TOOL_ID && $$LIBUSB1_INSTALL_NAME_TOOL_ID
