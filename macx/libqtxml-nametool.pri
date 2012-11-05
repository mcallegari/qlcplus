LIBQTXML_DIR      = QtXml.framework/Versions/4
LIBQTXML_FILE     = QtXml
LIBQTXML_FILEPATH = $$LIBQTXML_DIR/$$LIBQTXML_FILE

LIBQTXML_INSTALL_NAME_TOOL = install_name_tool -change $$LIBQTXML_FILEPATH \
            @executable_path/../$$LIBSDIR/$$LIBQTXML_DIR/$$LIBQTXML_FILE

contains(QT, xml) {
    !isEmpty(nametool.commands) {
        nametool.commands += "&&"
    }

    nametool.commands += $$LIBQTXML_INSTALL_NAME_TOOL $$OUTFILE
}

LIBQTXML.path   = $$INSTALLROOT/$$LIBSDIR/$$LIBQTXML_DIR
LIBQTXML.files += /Library/$$LIBSDIR/$$LIBQTXML_FILEPATH

LIBQTXML_INSTALL_NAME_TOOL_ID = install_name_tool -id @executable_path/../$$LIBSDIR/$$LIBQTXML_DIR/$$LIBQTXML_FILE \
                        $$INSTALLROOT/$$LIBSDIR/$$LIBQTXML_DIR/$$LIBQTXML_FILE
LIBQTXML_ID.path     = $$INSTALLROOT/$$LIBSDIR/$$LIBQTXML_DIR
LIBQTXML_ID.commands = $$LIBQTXML_INSTALL_NAME_TOOL_ID

