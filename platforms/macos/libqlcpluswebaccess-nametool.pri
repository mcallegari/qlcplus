LIBQLCWEBACCESS_FILE = libqlcpluswebaccess.1.dylib
LIBQLCWEBACCESS_INSTALL_NAME_TOOL = install_name_tool -change $$LIBQLCWEBACCESS_FILE \
            @executable_path/../$$LIBSDIR/$$LIBQLCWEBACCESS_FILE

contains(LIBS, -lqlcpluswebaccess) {
    !isEmpty(nametool.commands) {
        nametool.commands += "&&"
    }

    nametool.commands += $$LIBQLCWEBACCESS_INSTALL_NAME_TOOL $$OUTFILE
}

LIBQLCWEBACCESS_INSTALL_NAME_TOOL_ID = \
    install_name_tool -id @executable_path/../$$LIBSDIR/$$LIBQLCWEBACCESS_FILE \
    $$INSTALLROOT/$$LIBSDIR/$$LIBQLCWEBACCESS_FILE
LIBQLCWEBACCESS_ID.path     = $$INSTALLROOT/$$LIBSDIR/$$LIBQLCWEBACCESS_DIR
LIBQLCWEBACCESS_ID.commands = $$LIBQLCWEBACCESS_INSTALL_NAME_TOOL_ID
