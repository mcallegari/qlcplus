LIBPORTAUDIO_DIR      = $$system("pkg-config --variable libdir mad")
LIBPORTAUDIO_FILE     = libportaudio.2.dylib
LIBPORTAUDIO_FILEPATH = $$LIBPORTAUDIO_DIR/$$LIBPORTAUDIO_FILE

LIBPORTAUDIO_INSTALL_NAME_TOOL = install_name_tool -change $$LIBPORTAUDIO_FILEPATH \
                @executable_path/../$$LIBSDIR/$$LIBPORTAUDIO_FILE

contains(PKGCONFIG, mad) {
    !isEmpty(nametool.commands) {
        nametool.commands += "&&" 
    }

    nametool.commands += $$LIBPORTAUDIO_INSTALL_NAME_TOOL $$OUTFILE
}

LIBPORTAUDIO.path   = $$INSTALLROOT/$$LIBSDIR
LIBPORTAUDIO.files += $$LIBPORTAUDIO_FILEPATH

LIBPORTAUDIO_INSTALL_NAME_TOOL_ID = install_name_tool -id @executable_path/../$$LIBSDIR/$$LIBPORTAUDIO_FILE \
                $$INSTALLROOT/$$LIBSDIR/$$LIBPORTAUDIO_FILE
LIBPORTAUDIO_ID.path     = $$INSTALLROOT/$$LIBSDIR
LIBPORTAUDIO_ID.commands = $$LIBPORTAUDIO_INSTALL_NAME_TOOL_ID
