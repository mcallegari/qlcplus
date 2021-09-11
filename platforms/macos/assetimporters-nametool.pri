ASSETIMPORTERS_DIR  = $$(QTDIR)/plugins/assetimporters
assetimporters.path = $$INSTALLROOT/PlugIns/assetimporters

FORMATS = assimp
for(i, FORMATS):{
    FILE = lib$${i}.dylib
    assetimporters.files += $$ASSETIMPORTERS_DIR/$$FILE
    qtnametool.commands += && $$LIBQTCORE_INSTALL_NAME_TOOL $$INSTALLROOT/PlugIns/assetimporters/$$FILE
    qtnametool.commands += && $$LIBQTGUI_INSTALL_NAME_TOOL $$INSTALLROOT/PlugIns/assetimporters/$$FILE
    qtnametool.commands += && $$LIBQTNETWORK_INSTALL_NAME_TOOL $$INSTALLROOT/PlugIns/assetimporters/$$FILE
    #qtnametool.commands += && install_name_tool -id @executable_path/../PlugIns/$$FILE $$INSTALLROOT/PlugIns/$$FILE

    !isEmpty(nametool.commands) {
        nametool.commands += "&&"
    }

    nametool.commands += install_name_tool -change $$(QTDIR)/plugins/assetimporters/$$FILE \
                @executable_path/../PlugIns/assetimporters/$$FILE $$OUTFILE
}
