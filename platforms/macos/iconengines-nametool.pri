# Icon engine plugins
ICONENGINES_DIR  = $$(QTDIR)/plugins/iconengines
iconengines.path = $$INSTALLROOT/PlugIns/iconengines

FLAVORS = qsvgicon
for(i, FLAVORS):{
    FILE = lib$${i}.dylib
    iconengines.files += $$ICONENGINES_DIR/$$FILE
    qtnametool.commands += && $$LIBQTCORE_INSTALL_NAME_TOOL $$INSTALLROOT/PlugIns/iconengines/$$FILE

    !isEmpty(nametool.commands) {
        nametool.commands += "&&"
    }

    nametool.commands += install_name_tool -change $$(QTDIR)/plugins/iconengines/$$FILE \
                @executable_path/../PlugIns/iconengines/$$FILE $$OUTFILE
}
