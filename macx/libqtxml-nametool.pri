lessThan(QT_MAJOR_VERSION, 5) {
  LIBQTXML_DIR      = QtXml.framework/Versions/4
} else {
  LIBQTXML_DIR      = QtXml.framework/Versions/5
}
LIBQTXML_FILE     = QtXml
LIBQTXML_FILEPATH = $$LIBQTXML_DIR/$$LIBQTXML_FILE

lessThan(QT_MAJOR_VERSION, 5) {
LIBQTXML_INSTALL_NAME_TOOL = install_name_tool -change $$LIBQTXML_FILEPATH \
            @executable_path/../$$LIBSDIR/$$LIBQTXML_DIR/$$LIBQTXML_FILE
} else {
LIBQTXML_INSTALL_NAME_TOOL = install_name_tool -change $$(QTDIR)/lib/$$LIBQTXML_FILEPATH \
            @executable_path/../$$LIBSDIR/$$LIBQTXML_DIR/$$LIBQTXML_FILE
}

contains(QT, xml) {
    !isEmpty(nametool.commands) {
        nametool.commands += "&&"
    }

    nametool.commands += $$LIBQTXML_INSTALL_NAME_TOOL $$OUTFILE
}

LIBQTXML.path   = $$INSTALLROOT/$$LIBSDIR/$$LIBQTXML_DIR
lessThan(QT_MAJOR_VERSION, 5) {
  LIBQTXML.files += /Library/$$LIBSDIR/$$LIBQTXML_FILEPATH
} else {
  LIBQTXML.files += $$(QTDIR)/lib/$$LIBQTXML_FILEPATH
}
LIBQTXML_INSTALL_NAME_TOOL_ID = install_name_tool -id @executable_path/../$$LIBSDIR/$$LIBQTXML_DIR/$$LIBQTXML_FILE \
                        $$INSTALLROOT/$$LIBSDIR/$$LIBQTXML_DIR/$$LIBQTXML_FILE
LIBQTXML_ID.path     = $$INSTALLROOT/$$LIBSDIR/$$LIBQTXML_DIR
LIBQTXML_ID.commands = $$LIBQTXML_INSTALL_NAME_TOOL_ID

