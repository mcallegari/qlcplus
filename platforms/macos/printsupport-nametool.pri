PRINTSUPPORT_DIR  = $$(QTDIR)/plugins/printsupport
printsupport.path = $$INSTALLROOT/PlugIns/printsupport

FORMATS = cocoaprintersupport
for(i, FORMATS):{
    FILE = lib$${i}.dylib
    printsupport.files += $$PRINTSUPPORT_DIR/$$FILE
    qtnametool.commands += && $$LIBQTCORE_INSTALL_NAME_TOOL $$INSTALLROOT/PlugIns/printsupport/$$FILE
    qtnametool.commands += && $$LIBQTGUI_INSTALL_NAME_TOOL $$INSTALLROOT/PlugIns/printsupport/$$FILE
    qtnametool.commands += && $$LIBQTNETWORK_INSTALL_NAME_TOOL $$INSTALLROOT/PlugIns/printsupport/$$FILE
    #qtnametool.commands += && install_name_tool -id @executable_path/../PlugIns/$$FILE $$INSTALLROOT/PlugIns/$$FILE

    !isEmpty(nametool.commands) {
        nametool.commands += "&&"
    }

    nametool.commands += install_name_tool -change $$(QTDIR)/plugins/printsupport/$$FILE \
                @executable_path/../PlugIns/printsupport/$$FILE $$OUTFILE
}
