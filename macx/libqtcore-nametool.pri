LIBQTCORE_DIR      = QtCore.framework/Versions/4
LIBQTCORE_FILE     = QtCore
LIBQTCORE_FILEPATH = $$LIBQTCORE_DIR/$$LIBQTCORE_FILE

LIBQTCORE_INSTALL_NAME_TOOL = install_name_tool -change $$LIBQTCORE_FILEPATH \
            @executable_path/../$$LIBSDIR/$$LIBQTCORE_DIR/$$LIBQTCORE_FILE

contains(QT, core) {
    !isEmpty(nametool.commands) {
        nametool.commands += "&&"
    }

    nametool.commands += $$LIBQTCORE_INSTALL_NAME_TOOL $$OUTFILE
}

LIBQTCORE.path   = $$INSTALLROOT/$$LIBSDIR/$$LIBQTCORE_DIR
LIBQTCORE.files += /Library/$$LIBSDIR/$$LIBQTCORE_FILEPATH

LIBQTCORE_INSTALL_NAME_TOOL_ID = install_name_tool -id @executable_path/../$$LIBSDIR/$$LIBQTCORE_DIR/$$LIBQTCORE_FILE \
                        $$INSTALLROOT/$$LIBSDIR/$$LIBQTCORE_DIR/$$LIBQTCORE_FILE
LIBQTCORE_ID.path     = $$INSTALLROOT/$$LIBSDIR/$$LIBQTCORE_DIR
LIBQTCORE_ID.commands = $$LIBQTCORE_INSTALL_NAME_TOOL_ID

