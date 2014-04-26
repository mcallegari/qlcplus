lessThan(QT_MAJOR_VERSION, 5) {
  LIBQTCORE_DIR      = QtCore.framework/Versions/4
} else {
  LIBQTCORE_DIR      = QtCore.framework/Versions/5
}
LIBQTCORE_FILE     = QtCore
LIBQTCORE_FILEPATH = $$LIBQTCORE_DIR/$$LIBQTCORE_FILE

lessThan(QT_MAJOR_VERSION, 5) {
  LIBQTCORE_INSTALL_NAME_TOOL = install_name_tool -change $$LIBQTCORE_FILEPATH \
            @executable_path/../$$LIBSDIR/$$LIBQTCORE_DIR/$$LIBQTCORE_FILE
} else {
  LIBQTCORE_INSTALL_NAME_TOOL = install_name_tool -change $$(QTDIR)/lib/$$LIBQTCORE_FILEPATH \
            @executable_path/../$$LIBSDIR/$$LIBQTCORE_DIR/$$LIBQTCORE_FILE
}

contains(QT, core) {
    !isEmpty(nametool.commands) {
        nametool.commands += "&&"
    }

    nametool.commands += $$LIBQTCORE_INSTALL_NAME_TOOL $$OUTFILE
}

LIBQTCORE.path   = $$INSTALLROOT/$$LIBSDIR/$$LIBQTCORE_DIR
lessThan(QT_MAJOR_VERSION, 5) {
  LIBQTCORE.files += /Library/$$LIBSDIR/$$LIBQTCORE_FILEPATH
} else {
  message(Building with Qt: $$(QTDIR))
  LIBQTCORE.files += $$(QTDIR)/lib/$$LIBQTCORE_FILEPATH
}
LIBQTCORE_INSTALL_NAME_TOOL_ID = install_name_tool -id @executable_path/../$$LIBSDIR/$$LIBQTCORE_DIR/$$LIBQTCORE_FILE \
                        $$INSTALLROOT/$$LIBSDIR/$$LIBQTCORE_DIR/$$LIBQTCORE_FILE
LIBQTCORE_ID.path     = $$INSTALLROOT/$$LIBSDIR/$$LIBQTCORE_DIR
LIBQTCORE_ID.commands = $$LIBQTCORE_INSTALL_NAME_TOOL_ID

