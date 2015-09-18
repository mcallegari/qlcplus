LIBQTQML_DIR      = QtQml.framework/Versions/5
LIBQTQML_FILE     = QtQml
LIBQTQML_FILEPATH = $$LIBQTQML_DIR/$$LIBQTQML_FILE

LIBQTQML_INSTALL_NAME_TOOL = install_name_tool -change $$(QTDIR)/lib/$$LIBQTQML_FILEPATH \
            @executable_path/../$$LIBSDIR/$$LIBQTQML_DIR/$$LIBQTQML_FILE

contains(QT, qml) {
    !isEmpty(nametool.commands) {
        nametool.commands += "&&"
    }

    nametool.commands += $$LIBQTQML_INSTALL_NAME_TOOL $$OUTFILE
}

LIBQTQML.path   = $$INSTALLROOT/$$LIBSDIR/$$LIBQTQML_DIR
LIBQTQML.files += $$(QTDIR)/lib/$$LIBQTQML_FILEPATH

LIBQTQML_INSTALL_NAME_TOOL_ID = install_name_tool -id @executable_path/../$$LIBSDIR/$$LIBQTQML_DIR/$$LIBQTQML_FILE \
                        $$INSTALLROOT/$$LIBSDIR/$$LIBQTQML_DIR/$$LIBQTQML_FILE
LIBQTQML_ID.path     = $$INSTALLROOT/$$LIBSDIR/$$LIBQTQML_DIR
LIBQTQML_ID.commands = $$LIBQTQML_INSTALL_NAME_TOOL_ID

