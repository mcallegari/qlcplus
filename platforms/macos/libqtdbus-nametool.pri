LIBQTDBUS_DIR      = QtDBus.framework/Versions/5
LIBQTDBUS_FILE     = QtDBus
LIBQTDBUS_FILEPATH = $$LIBQTDBUS_DIR/$$LIBQTDBUS_FILE

LIBQTDBUS_INSTALL_NAME_TOOL = install_name_tool -change $$(QTDIR)/lib/$$LIBQTDBUS_FILEPATH \
            @executable_path/../$$LIBSDIR/$$LIBQTDBUS_DIR/$$LIBQTDBUS_FILE

contains(QT, dbus) {
    !isEmpty(nametool.commands) {
        nametool.commands += "&&"
    }

    nametool.commands += $$LIBQTDBUS_INSTALL_NAME_TOOL $$OUTFILE
}

LIBQTDBUS.path   = $$INSTALLROOT/$$LIBSDIR/$$LIBQTDBUS_DIR
LIBQTDBUS.files += $$(QTDIR)/lib/$$LIBQTDBUS_FILEPATH

LIBQTDBUS_INSTALL_NAME_TOOL_ID = install_name_tool -id @executable_path/../$$LIBSDIR/$$LIBQTDBUS_DIR/$$LIBQTDBUS_FILE \
                        $$INSTALLROOT/$$LIBSDIR/$$LIBQTDBUS_DIR/$$LIBQTDBUS_FILE
LIBQTDBUS_ID.path     = $$INSTALLROOT/$$LIBSDIR/$$LIBQTDBUS_DIR
LIBQTDBUS_ID.commands = $$LIBQTDBUS_INSTALL_NAME_TOOL_ID

