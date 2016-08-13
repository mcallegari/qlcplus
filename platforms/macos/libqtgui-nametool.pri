lessThan(QT_MAJOR_VERSION, 5) {
  LIBQTGUI_DIR      = QtGui.framework/Versions/4
} else {
  LIBQTGUI_DIR      = QtGui.framework/Versions/5
}
LIBQTGUI_FILE     = QtGui
LIBQTGUI_FILEPATH = $$LIBQTGUI_DIR/$$LIBQTGUI_FILE

lessThan(QT_MAJOR_VERSION, 5) {
LIBQTGUI_INSTALL_NAME_TOOL = install_name_tool -change $$LIBQTGUI_FILEPATH \
            @executable_path/../$$LIBSDIR/$$LIBQTGUI_DIR/$$LIBQTGUI_FILE
} else {
LIBQTGUI_INSTALL_NAME_TOOL = install_name_tool -change $$(QTDIR)/lib/$$LIBQTGUI_FILEPATH \
            @executable_path/../$$LIBSDIR/$$LIBQTGUI_DIR/$$LIBQTGUI_FILE
}
contains(QT, gui) {
    !isEmpty(nametool.commands) {
        nametool.commands += "&&"
    }

    nametool.commands += $$LIBQTGUI_INSTALL_NAME_TOOL $$OUTFILE
}

LIBQTGUI.path   = $$INSTALLROOT/$$LIBSDIR/$$LIBQTGUI_DIR
lessThan(QT_MAJOR_VERSION, 5) {
  LIBQTGUI.files += /Library/$$LIBSDIR/$$LIBQTGUI_FILEPATH
  QTMENU.files += /Library/$$LIBSDIR/$$LIBQTGUI_DIR/Resources/*
} else {
  LIBQTGUI.files += $$(QTDIR)/lib/$$LIBQTGUI_FILEPATH
  QTMENU.files += $$(QTDIR)/lib/$$LIBQTGUI_DIR/Resources/*
}
QTMENU.path   = $$INSTALLROOT/$$DATADIR

LIBQTGUI_INSTALL_NAME_TOOL_ID = install_name_tool -id @executable_path/../$$LIBSDIR/$$LIBQTGUI_DIR/$$LIBQTGUI_FILE \
            $$INSTALLROOT/$$LIBSDIR/$$LIBQTGUI_DIR/$$LIBQTGUI_FILE
LIBQTGUI_ID.path     = $$INSTALLROOT/$$LIBSDIR/$$LIBQTGUI_DIR
LIBQTGUI_ID.commands = $$LIBQTGUI_INSTALL_NAME_TOOL_ID
