# Image format plugins depend on various Qt libraries
IMAGEFORMATS_DIR  = $$(QTDIR)/plugins/imageformats
imageformats.path = $$INSTALLROOT/PlugIns/imageformats

FORMATS = qjpeg qgif qsvg
for(i, FORMATS):{
    FILE = lib$${i}.dylib
    imageformats.files += $$IMAGEFORMATS_DIR/$$FILE
    qtnametool.commands += && $$LIBQTCORE_INSTALL_NAME_TOOL $$INSTALLROOT/PlugIns/imageformats/$$FILE
    qtnametool.commands += && $$LIBQTGUI_INSTALL_NAME_TOOL $$INSTALLROOT/PlugIns/imageformats/$$FILE
    qtnametool.commands += && $$LIBQTXML_INSTALL_NAME_TOOL $$INSTALLROOT/PlugIns/imageformats/$$FILE
    qtnametool.commands += && $$LIBQTNETWORK_INSTALL_NAME_TOOL $$INSTALLROOT/PlugIns/imageformats/$$FILE
    #qtnametool.commands += && install_name_tool -id @executable_path/../PlugIns/$$FILE $$INSTALLROOT/PlugIns/$$FILE

    !isEmpty(nametool.commands) {
        nametool.commands += "&&"
    }

    nametool.commands += install_name_tool -change $$(QTDIR)/plugins/imageformats/$$FILE \
                @executable_path/../PlugIns/imageformats/$$FILE $$OUTFILE
}
