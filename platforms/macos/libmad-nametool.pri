LIBMAD_DIR      = $$system("pkg-config --variable libdir mad")
LIBMAD_FILE     = libmad.0.dylib
LIBMAD_FILEPATH = $$LIBMAD_DIR/$$LIBMAD_FILE

LIBMAD_INSTALL_NAME_TOOL = install_name_tool -change $$LIBMAD_FILEPATH \
                @executable_path/../$$LIBSDIR/$$LIBMAD_FILE

contains(PKGCONFIG, mad) {
    !isEmpty(nametool.commands) {
        nametool.commands += "&&" 
    }

    nametool.commands += $$LIBMAD_INSTALL_NAME_TOOL $$OUTFILE
}

LIBMAD.path   = $$INSTALLROOT/$$LIBSDIR
LIBMAD.files += $$LIBMAD_FILEPATH

LIBMAD_INSTALL_NAME_TOOL_ID = install_name_tool -id @executable_path/../$$LIBSDIR/$$LIBMAD_FILE \
                $$INSTALLROOT/$$LIBSDIR/$$LIBMAD_FILE
LIBMAD_ID.path     = $$INSTALLROOT/$$LIBSDIR
LIBMAD_ID.commands = $$LIBMAD_INSTALL_NAME_TOOL_ID
