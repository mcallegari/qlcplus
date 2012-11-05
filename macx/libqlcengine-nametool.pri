LIBQLCENGINE_FILE = libqlcengine.1.dylib
LIBQLCENGINE_INSTALL_NAME_TOOL = install_name_tool -change $$LIBQLCENGINE_FILE \
            @executable_path/../$$LIBSDIR/$$LIBQLCENGINE_FILE

contains(LIBS, -lqlcengine) {
    !isEmpty(nametool.commands) {
        nametool.commands += "&&"
    }

    nametool.commands += $$LIBQLCENGINE_INSTALL_NAME_TOOL $$OUTFILE
}

LIBQLCENGINE_INSTALL_NAME_TOOL_ID = \
    install_name_tool -id @executable_path/../$$LIBSDIR/$$LIBQLCENGINE_FILE \
    $$INSTALLROOT/$$LIBSDIR/$$LIBQLCENGINE_FILE
LIBQLCENGINE_ID.path     = $$INSTALLROOT/$$LIBSDIR/$$LIBQLCENGINE_DIR
LIBQLCENGINE_ID.commands = $$LIBQLCENGINE_INSTALL_NAME_TOOL_ID
