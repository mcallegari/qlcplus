lessThan(QT_MAJOR_VERSION, 5) {
  LIBQTSCRIPT_DIR      = QtScript.framework/Versions/4
} else {
  LIBQTSCRIPT_DIR      = QtScript.framework/Versions/5
}
LIBQTSCRIPT_FILE     = QtScript
LIBQTSCRIPT_FILEPATH = $$LIBQTSCRIPT_DIR/$$LIBQTSCRIPT_FILE

lessThan(QT_MAJOR_VERSION, 5) {
LIBQTSCRIPT_INSTALL_NAME_TOOL = install_name_tool -change $$LIBQTSCRIPT_FILEPATH \
            @executable_path/../$$LIBSDIR/$$LIBQTSCRIPT_DIR/$$LIBQTSCRIPT_FILE
} else {
LIBQTSCRIPT_INSTALL_NAME_TOOL = install_name_tool -change $$(QTDIR)/lib/$$LIBQTSCRIPT_FILEPATH \
            @executable_path/../$$LIBSDIR/$$LIBQTSCRIPT_DIR/$$LIBQTSCRIPT_FILE
}

contains(QT, script) {
    !isEmpty(nametool.commands) {
        nametool.commands += "&&"
    }

    nametool.commands += $$LIBQTSCRIPT_INSTALL_NAME_TOOL $$OUTFILE
}

LIBQTSCRIPT.path   = $$INSTALLROOT/$$LIBSDIR/$$LIBQTSCRIPT_DIR
lessThan(QT_MAJOR_VERSION, 5) {
  LIBQTSCRIPT.files += /Library/$$LIBSDIR/$$LIBQTSCRIPT_FILEPATH
} else {
  LIBQTSCRIPT.files += $$(QTDIR)/lib/$$LIBQTSCRIPT_FILEPATH
}
LIBQTSCRIPT_INSTALL_NAME_TOOL_ID = install_name_tool -id @executable_path/../$$LIBSDIR/$$LIBQTSCRIPT_DIR/$$LIBQTSCRIPT_FILE \
                        $$INSTALLROOT/$$LIBSDIR/$$LIBQTSCRIPT_DIR/$$LIBQTSCRIPT_FILE
LIBQTSCRIPT_ID.path     = $$INSTALLROOT/$$LIBSDIR/$$LIBQTSCRIPT_DIR
LIBQTSCRIPT_ID.commands = $$LIBQTSCRIPT_INSTALL_NAME_TOOL_ID

