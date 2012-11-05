# Image format plugins depend on various Qt libraries
IMAGEFORMATS_DIR  = /Developer/Applications/Qt/plugins/imageformats
imageformats.path = $$INSTALLROOT/PlugIns

FORMATS = qjpeg qgif
for(i, FORMATS):{
    FILE = lib$${i}.dylib
    imageformats.files += $$IMAGEFORMATS_DIR/$$FILE
    qtnametool.commands += && $$LIBQTCORE_INSTALL_NAME_TOOL $$INSTALLROOT/PlugIns/$$FILE
    qtnametool.commands += && $$LIBQTGUI_INSTALL_NAME_TOOL $$INSTALLROOT/PlugIns/$$FILE
    qtnametool.commands += && $$LIBQTXML_INSTALL_NAME_TOOL $$INSTALLROOT/PlugIns/$$FILE
    qtnametool.commands += && $$LIBQTNETWORK_INSTALL_NAME_TOOL $$INSTALLROOT/PlugIns/$$FILE
    qtnametool.commands += && install_name_tool -id @executable_path/../PlugIns/$$FILE $$INSTALLROOT/PlugIns/$$FILE

    !isEmpty(nametool.commands) {
        nametool.commands += "&&"
    }

    nametool.commands += install_name_tool -change $$FILE \
                @executable_path/../PlugIns/$$FILE $$OUTFILE
}
