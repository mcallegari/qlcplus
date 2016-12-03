LIBQTQUICK_DIR      = QtQuick.framework/Versions/5
LIBQTQUICK_FILE     = QtQuick
LIBQTQUICK_FILEPATH = $$LIBQTQUICK_DIR/$$LIBQTQUICK_FILE

LIBQTQUICK_INSTALL_NAME_TOOL = install_name_tool -change $$(QTDIR)/lib/$$LIBQTQUICK_FILEPATH \
            @executable_path/../$$LIBSDIR/$$LIBQTQUICK_DIR/$$LIBQTQUICK_FILE

contains(QT, quick) {
    !isEmpty(nametool.commands) {
        nametool.commands += "&&"
    }

    nametool.commands += $$LIBQTQUICK_INSTALL_NAME_TOOL $$OUTFILE
}

LIBQTQUICK.path   = $$INSTALLROOT/$$LIBSDIR/$$LIBQTQUICK_DIR
LIBQTQUICK.files += $$(QTDIR)/lib/$$LIBQTQUICK_FILEPATH

LIBQTQUICK_INSTALL_NAME_TOOL_ID = install_name_tool -id @executable_path/../$$LIBSDIR/$$LIBQTQUICK_DIR/$$LIBQTQUICK_FILE \
                        $$INSTALLROOT/$$LIBSDIR/$$LIBQTQUICK_DIR/$$LIBQTQUICK_FILE
LIBQTQUICK_ID.path     = $$INSTALLROOT/$$LIBSDIR/$$LIBQTQUICK_DIR
LIBQTQUICK_ID.commands = $$LIBQTQUICK_INSTALL_NAME_TOOL_ID

