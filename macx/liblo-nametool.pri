LIBLO_DIR      = $$system("pkg-config --variable libdir liblo")
LIBLO_FILE     = liblo.7.dylib
LIBLO_FILEPATH = $$LIBLO_DIR/$$LIBLO_FILE

LIBLO_INSTALL_NAME_TOOL = install_name_tool -change $$LIBLO_FILEPATH \
                @executable_path/../$$LIBSDIR/$$LIBLO_FILE

contains(PKGCONFIG, liblo) {
    !isEmpty(nametool.commands) {
        nametool.commands += "&&" 
    }

    nametool.commands += $$LIBLO_INSTALL_NAME_TOOL $$OUTFILE
}

LIBLO.path   = $$INSTALLROOT/$$LIBSDIR
LIBLO.files += $$LIBLO_FILEPATH

LIBLO_INSTALL_NAME_TOOL_ID = install_name_tool -id @executable_path/../$$LIBSDIR/$$LIBLO_FILE \
                $$INSTALLROOT/$$LIBSDIR/$$LIBLO_FILE
LIBLO_ID.path     = $$INSTALLROOT/$$LIBSDIR
LIBLO_ID.commands = $$LIBLO_INSTALL_NAME_TOOL_ID
