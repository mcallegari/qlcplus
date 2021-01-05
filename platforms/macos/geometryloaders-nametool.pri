GEOMETRYLOADERS_DIR  = $$(QTDIR)/plugins/geometryloaders
geometryloaders.path = $$INSTALLROOT/PlugIns/geometryloaders

FORMATS = defaultgeometryloader
for(i, FORMATS):{
    FILE = lib$${i}.dylib
    geometryloaders.files += $$GEOMETRYLOADERS_DIR/$$FILE
    qtnametool.commands += && $$LIBQTCORE_INSTALL_NAME_TOOL $$INSTALLROOT/PlugIns/geometryloaders/$$FILE
    qtnametool.commands += && $$LIBQTGUI_INSTALL_NAME_TOOL $$INSTALLROOT/PlugIns/geometryloaders/$$FILE
    qtnametool.commands += && $$LIBQTNETWORK_INSTALL_NAME_TOOL $$INSTALLROOT/PlugIns/geometryloaders/$$FILE
    #qtnametool.commands += && install_name_tool -id @executable_path/../PlugIns/$$FILE $$INSTALLROOT/PlugIns/$$FILE

    !isEmpty(nametool.commands) {
        nametool.commands += "&&"
    }

    nametool.commands += install_name_tool -change $$(QTDIR)/plugins/geometryloaders/$$FILE \
                @executable_path/../PlugIns/geometryloaders/$$FILE $$OUTFILE
}
