# Basic platform plugin
PLATFORMSPLUGIN_DIR  = $$(QTDIR)/plugins/platforms
platformplugins.path = $$INSTALLROOT/MacOS/platforms

FLAVORS = qcocoa
for(i, FLAVORS):{
    FILE = lib$${i}.dylib
    platformplugins.files += $$PLATFORMSPLUGIN_DIR/$$FILE
    qtnametool.commands += && $$LIBQTCORE_INSTALL_NAME_TOOL $$INSTALLROOT/MacOS/platforms/$$FILE
    qtnametool.commands += && $$LIBQTGUI_INSTALL_NAME_TOOL $$INSTALLROOT/MacOS/platforms/$$FILE
    qtnametool.commands += && $$LIBQTWIDGETS_INSTALL_NAME_TOOL $$INSTALLROOT/MacOS/platforms/$$FILE
    qtnametool.commands += && $$LIBQTPRINTSUPPORT_INSTALL_NAME_TOOL $$INSTALLROOT/MacOS/platforms/$$FILE
    qtnametool.commands += && install_name_tool -id @executable_path/../MacOS/platforms/$$FILE $$INSTALLROOT/MacOS/platforms/$$FILE

    !isEmpty(nametool.commands) {
        nametool.commands += "&&"
    }

    nametool.commands += install_name_tool -change $$(QTDIR)/plugins/platforms/$$FILE \
                @executable_path/../MacOS/platforms/$$FILE $$OUTFILE
}
