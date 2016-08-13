LIBQTSVG_DIR      = QtSvg.framework/Versions/5
LIBQTSVG_FILE     = QtSvg
LIBQTSVG_FILEPATH = $$LIBQTSVG_DIR/$$LIBQTSVG_FILE

LIBQTSVG_INSTALL_NAME_TOOL = install_name_tool -change $$(QTDIR)/lib/$$LIBQTSVG_FILEPATH \
            @executable_path/../$$LIBSDIR/$$LIBQTSVG_DIR/$$LIBQTSVG_FILE

contains(QT, svg) {
    !isEmpty(nametool.commands) {
        nametool.commands += "&&"
    }

    nametool.commands += $$LIBQTSVG_INSTALL_NAME_TOOL $$OUTFILE
}

LIBQTSVG.path   = $$INSTALLROOT/$$LIBSDIR/$$LIBQTSVG_DIR
LIBQTSVG.files += $$(QTDIR)/lib/$$LIBQTSVG_FILEPATH

LIBQTSVG_INSTALL_NAME_TOOL_ID = install_name_tool -id @executable_path/../$$LIBSDIR/$$LIBQTSVG_DIR/$$LIBQTSVG_FILE \
                        $$INSTALLROOT/$$LIBSDIR/$$LIBQTSVG_DIR/$$LIBQTSVG_FILE
LIBQTSVG_ID.path     = $$INSTALLROOT/$$LIBSDIR/$$LIBQTSVG_DIR
LIBQTSVG_ID.commands = $$LIBQTSVG_INSTALL_NAME_TOOL_ID

