# Style plugins
STYLES_DIR  = $$(QTDIR)/plugins/styles
styles.path = $$INSTALLROOT/PlugIns/styles

FLAVORS = qmacstyle
for(i, FLAVORS):{
    FILE = lib$${i}.dylib
    styles.files += $$STYLES_DIR/$$FILE
    qtnametool.commands += && $$LIBQTCORE_INSTALL_NAME_TOOL $$INSTALLROOT/PlugIns/styles/$$FILE

    !isEmpty(nametool.commands) {
        nametool.commands += "&&"
    }

    nametool.commands += install_name_tool -change $$(QTDIR)/plugins/styles/$$FILE \
                @executable_path/../PlugIns/styles/$$FILE $$OUTFILE
}
