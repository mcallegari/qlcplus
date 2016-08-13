LIBQLCUI_FILE = libqlcplusui.1.dylib
LIBQLCUI_INSTALL_NAME_TOOL = install_name_tool -change $$LIBQLCUI_FILE \
            @executable_path/../$$LIBSDIR/$$LIBQLCUI_FILE

contains(LIBS, -lqlcplusui) {
    !isEmpty(nametool.commands) {
        nametool.commands += "&&"
    }

    nametool.commands += $$LIBQLCUI_INSTALL_NAME_TOOL $$OUTFILE
}

LIBQLCUI_INSTALL_NAME_TOOL_ID = \
    install_name_tool -id @executable_path/../$$LIBSDIR/$$LIBQLCUI_FILE \
    $$INSTALLROOT/$$LIBSDIR/$$LIBQLCUI_FILE
LIBQLCUI_ID.path     = $$INSTALLROOT/$$LIBSDIR/$$LIBQLCUI_DIR
LIBQLCUI_ID.commands = $$LIBQLCUI_INSTALL_NAME_TOOL_ID
