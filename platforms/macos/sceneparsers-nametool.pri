SCENEPARSERS_DIR  = $$(QTDIR)/plugins/sceneparsers
sceneparsers.path = $$INSTALLROOT/PlugIns/sceneparsers

FORMATS = assimpsceneimport
for(i, FORMATS):{
    FILE = lib$${i}.dylib
    sceneparsers.files += $$SCENEPARSERS_DIR/$$FILE
    qtnametool.commands += && $$LIBQTCORE_INSTALL_NAME_TOOL $$INSTALLROOT/PlugIns/sceneparsers/$$FILE
    qtnametool.commands += && $$LIBQTGUI_INSTALL_NAME_TOOL $$INSTALLROOT/PlugIns/sceneparsers/$$FILE
    qtnametool.commands += && $$LIBQTNETWORK_INSTALL_NAME_TOOL $$INSTALLROOT/PlugIns/sceneparsers/$$FILE
    #qtnametool.commands += && install_name_tool -id @executable_path/../PlugIns/$$FILE $$INSTALLROOT/PlugIns/$$FILE

    !isEmpty(nametool.commands) {
        nametool.commands += "&&"
    }

    nametool.commands += install_name_tool -change $$(QTDIR)/plugins/sceneparsers/$$FILE \
                @executable_path/../PlugIns/sceneparsers/$$FILE $$OUTFILE
}
