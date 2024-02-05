RENDERERS_DIR  = $$(QTDIR)/plugins/renderers
renderers.path = $$INSTALLROOT/PlugIns/renderers

BACKENDS = openglrenderer
for(i, BACKENDS):{
    FILE = lib$${i}.dylib
    renderers.files += $$RENDERERS_DIR/$$FILE
    qtnametool.commands += && $$LIBQTCORE_INSTALL_NAME_TOOL $$INSTALLROOT/PlugIns/renderers/$$FILE
    qtnametool.commands += && $$LIBQTGUI_INSTALL_NAME_TOOL $$INSTALLROOT/PlugIns/renderers/$$FILE
    qtnametool.commands += && $$LIBQTNETWORK_INSTALL_NAME_TOOL $$INSTALLROOT/PlugIns/renderers/$$FILE

    !isEmpty(nametool.commands) {
        nametool.commands += "&&"
    }

    nametool.commands += install_name_tool -change $$(QTDIR)/plugins/renderers/$$FILE \
                @executable_path/../PlugIns/renderers/$$FILE $$OUTFILE
}
