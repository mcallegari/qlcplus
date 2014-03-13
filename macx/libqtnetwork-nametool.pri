lessThan(QT_MAJOR_VERSION, 5) {
  LIBQTNETWORK_DIR      = QtNetwork.framework/Versions/4
} else {
  LIBQTNETWORK_DIR      = QtNetwork.framework/Versions/5
}
LIBQTNETWORK_FILE     = QtNetwork
LIBQTNETWORK_FILEPATH = $$LIBQTNETWORK_DIR/$$LIBQTNETWORK_FILE

lessThan(QT_MAJOR_VERSION, 5) {
LIBQTNETWORK_INSTALL_NAME_TOOL = install_name_tool -change $$LIBQTNETWORK_FILEPATH \
        @executable_path/../$$LIBSDIR/$$LIBQTNETWORK_DIR/$$LIBQTNETWORK_FILE
} else {
LIBQTNETWORK_INSTALL_NAME_TOOL = install_name_tool -change $$(QTDIR)/lib/$$LIBQTNETWORK_FILEPATH \
        @executable_path/../$$LIBSDIR/$$LIBQTNETWORK_DIR/$$LIBQTNETWORK_FILE
}

contains(QT, network) {
    !isEmpty(nametool.commands) {
        nametool.commands += "&&"
    }

    nametool.commands += $$LIBQTNETWORK_INSTALL_NAME_TOOL $$OUTFILE
}

LIBQTNETWORK.path   = $$INSTALLROOT/$$LIBSDIR/$$LIBQTNETWORK_DIR

lessThan(QT_MAJOR_VERSION, 5) {
  LIBQTNETWORK.files += /Library/$$LIBSDIR/$$LIBQTNETWORK_FILEPATH
} else {
  LIBQTNETWORK.files += $$(QTDIR)/lib/$$LIBQTNETWORK_FILEPATH
}

LIBQTNETWORK_INSTALL_NAME_TOOL_ID = install_name_tool -id @executable_path/../$$LIBSDIR/$$LIBQTNETWORK_DIR/$$LIBQTNETWORK_FILE \
                        $$INSTALLROOT/$$LIBSDIR/$$LIBQTNETWORK_DIR/$$LIBQTNETWORK_FILE
LIBQTNETWORK_ID.path     = $$INSTALLROOT/$$LIBSDIR/$$LIBQTNETWORK_DIR
LIBQTNETWORK_ID.commands = $$LIBQTNETWORK_INSTALL_NAME_TOOL_ID

