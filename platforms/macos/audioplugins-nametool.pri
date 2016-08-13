# Basic audio plugins
AUDIOPLUGIN_DIR  = $$(QTDIR)/plugins/audio
audioplugins.path = $$INSTALLROOT/PlugIns/audio

FLAVORS = qtaudio_coreaudio
for(i, FLAVORS):{
    FILE = lib$${i}.dylib
    audioplugins.files += $$AUDIOPLUGIN_DIR/$$FILE
    qtnametool.commands += && $$LIBQTCORE_INSTALL_NAME_TOOL $$INSTALLROOT/PlugIns/audio/$$FILE
    qtnametool.commands += && $$LIBQTGUI_INSTALL_NAME_TOOL $$INSTALLROOT/PlugIns/audio/$$FILE
    qtnametool.commands += && $$LIBQTNETWORK_INSTALL_NAME_TOOL $$INSTALLROOT/PlugIns/audio/$$FILE
    qtnametool.commands += && $$LIBQTMULTIMEDIA_INSTALL_NAME_TOOL $$INSTALLROOT/PlugIns/audio/$$FILE
    #qtnametool.commands += && install_name_tool -id @executable_path/../PlugIns/audio/$$FILE $$INSTALLROOT/PlugIns/audio/$$FILE

    !isEmpty(nametool.commands) {
        nametool.commands += "&&"
    }

    nametool.commands += install_name_tool -change $$(QTDIR)/plugins/audio/$$FILE \
                @executable_path/../PlugIns/audio/$$FILE $$OUTFILE
}
