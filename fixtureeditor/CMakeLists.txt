set(module_name "qlcplus-fixtureeditor")

set(TS_FILES
    fixtureeditor_fi_FI.ts
    fixtureeditor_fr_FR.ts
    fixtureeditor_de_DE.ts
    fixtureeditor_es_ES.ts
    fixtureeditor_it_IT.ts
    fixtureeditor_nl_NL.ts
    fixtureeditor_cz_CZ.ts
    fixtureeditor_pt_BR.ts
    fixtureeditor_ca_ES.ts
    fixtureeditor_ja_JP.ts
)

if(QT_VERSION_MAJOR GREATER 5)
    qt_add_translation(QM_FILES ${TS_FILES})
else()
    qt5_add_translation(QM_FILES ${TS_FILES})
endif()

add_executable(${module_name} WIN32
    ../ui/src/aboutbox.cpp ../ui/src/aboutbox.h ../ui/src/aboutbox.ui
    ../ui/src/apputil.cpp ../ui/src/apputil.h
    ../ui/src/docbrowser.cpp ../ui/src/docbrowser.h
    addchannelsdialog.cpp addchannelsdialog.h addchannelsdialog.ui
    app.cpp app.h
    capabilitywizard.cpp capabilitywizard.h capabilitywizard.ui
    editchannel.cpp editchannel.h editchannel.ui
    edithead.cpp edithead.h edithead.ui
    editmode.cpp editmode.h editmode.ui
    editphysical.cpp editphysical.h editphysical.ui
    fixtureeditor.cpp fixtureeditor.h fixtureeditor.ui
    main.cpp
    util.h
    ${QM_FILES}
)

target_include_directories(${module_name} PRIVATE
    ../engine/src
    ../plugins/interfaces
    ../ui/src
)

target_link_libraries(${module_name} PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
    qlcplusengine
)


# Resources:
set_source_files_properties("../ui/src/../../resources/icons/png/add_dump.png"
    PROPERTIES QT_RESOURCE_ALIAS "add_dump.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/animation.png"
    PROPERTIES QT_RESOURCE_ALIAS "animation.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/attach.png"
    PROPERTIES QT_RESOURCE_ALIAS "attach.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/audio.png"
    PROPERTIES QT_RESOURCE_ALIAS "audio.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/audioinput.png"
    PROPERTIES QT_RESOURCE_ALIAS "audioinput.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/autostart.png"
    PROPERTIES QT_RESOURCE_ALIAS "autostart.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/back.png"
    PROPERTIES QT_RESOURCE_ALIAS "back.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/beam.png"
    PROPERTIES QT_RESOURCE_ALIAS "beam.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/blackout.png"
    PROPERTIES QT_RESOURCE_ALIAS "blackout.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/blind.png"
    PROPERTIES QT_RESOURCE_ALIAS "blind.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/button.png"
    PROPERTIES QT_RESOURCE_ALIAS "button.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/buttonmatrix.png"
    PROPERTIES QT_RESOURCE_ALIAS "buttonmatrix.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/chaser.png"
    PROPERTIES QT_RESOURCE_ALIAS "chaser.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/check.png"
    PROPERTIES QT_RESOURCE_ALIAS "check.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/checkbox_empty.png"
    PROPERTIES QT_RESOURCE_ALIAS "checkbox_empty.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/checkbox_full.png"
    PROPERTIES QT_RESOURCE_ALIAS "checkbox_full.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/clock.png"
    PROPERTIES QT_RESOURCE_ALIAS "clock.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/collection.png"
    PROPERTIES QT_RESOURCE_ALIAS "collection.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/color.png"
    PROPERTIES QT_RESOURCE_ALIAS "color.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/colorwheel.png"
    PROPERTIES QT_RESOURCE_ALIAS "colorwheel.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/configure.png"
    PROPERTIES QT_RESOURCE_ALIAS "configure.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/cuelist.png"
    PROPERTIES QT_RESOURCE_ALIAS "cuelist.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/current.png"
    PROPERTIES QT_RESOURCE_ALIAS "current.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/delete.png"
    PROPERTIES QT_RESOURCE_ALIAS "delete.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/design.png"
    PROPERTIES QT_RESOURCE_ALIAS "design.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/detach.png"
    PROPERTIES QT_RESOURCE_ALIAS "detach.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/dimmer.png"
    PROPERTIES QT_RESOURCE_ALIAS "dimmer.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/diptool.png"
    PROPERTIES QT_RESOURCE_ALIAS "diptool.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/down.png"
    PROPERTIES QT_RESOURCE_ALIAS "down.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/ds_border.png"
    PROPERTIES QT_RESOURCE_ALIAS "ds_border.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/ds_bottom.png"
    PROPERTIES QT_RESOURCE_ALIAS "ds_bottom.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/ds_off.png"
    PROPERTIES QT_RESOURCE_ALIAS "ds_off.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/ds_on.png"
    PROPERTIES QT_RESOURCE_ALIAS "ds_on.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/ds_top.png"
    PROPERTIES QT_RESOURCE_ALIAS "ds_top.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/edit.png"
    PROPERTIES QT_RESOURCE_ALIAS "edit.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/edit_add.png"
    PROPERTIES QT_RESOURCE_ALIAS "edit_add.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/edit_remove.png"
    PROPERTIES QT_RESOURCE_ALIAS "edit_remove.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/editclear.png"
    PROPERTIES QT_RESOURCE_ALIAS "editclear.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/editcopy.png"
    PROPERTIES QT_RESOURCE_ALIAS "editcopy.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/editcopyall.png"
    PROPERTIES QT_RESOURCE_ALIAS "editcopyall.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/editcut.png"
    PROPERTIES QT_RESOURCE_ALIAS "editcut.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/editdelete.png"
    PROPERTIES QT_RESOURCE_ALIAS "editdelete.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/editpaste.png"
    PROPERTIES QT_RESOURCE_ALIAS "editpaste.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/effect.png"
    PROPERTIES QT_RESOURCE_ALIAS "effect.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/efx.png"
    PROPERTIES QT_RESOURCE_ALIAS "efx.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/exit.png"
    PROPERTIES QT_RESOURCE_ALIAS "exit.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/expand.png"
    PROPERTIES QT_RESOURCE_ALIAS "expand.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/fade.png"
    PROPERTIES QT_RESOURCE_ALIAS "fade.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/fan.png"
    PROPERTIES QT_RESOURCE_ALIAS "fan.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/fileclose.png"
    PROPERTIES QT_RESOURCE_ALIAS "fileclose.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/fileexport.png"
    PROPERTIES QT_RESOURCE_ALIAS "fileexport.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/fileimport.png"
    PROPERTIES QT_RESOURCE_ALIAS "fileimport.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/filenew.png"
    PROPERTIES QT_RESOURCE_ALIAS "filenew.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/fileopen.png"
    PROPERTIES QT_RESOURCE_ALIAS "fileopen.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/filesave.png"
    PROPERTIES QT_RESOURCE_ALIAS "filesave.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/filesaveas.png"
    PROPERTIES QT_RESOURCE_ALIAS "filesaveas.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/fixture.png"
    PROPERTIES QT_RESOURCE_ALIAS "fixture.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/flash.png"
    PROPERTIES QT_RESOURCE_ALIAS "flash.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/flower.png"
    PROPERTIES QT_RESOURCE_ALIAS "flower.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/folder.png"
    PROPERTIES QT_RESOURCE_ALIAS "folder.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/fontcolor.png"
    PROPERTIES QT_RESOURCE_ALIAS "fontcolor.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/fonts.png"
    PROPERTIES QT_RESOURCE_ALIAS "fonts.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/forward.png"
    PROPERTIES QT_RESOURCE_ALIAS "forward.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/frame.png"
    PROPERTIES QT_RESOURCE_ALIAS "frame.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/frameraised.png"
    PROPERTIES QT_RESOURCE_ALIAS "frameraised.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/framesunken.png"
    PROPERTIES QT_RESOURCE_ALIAS "framesunken.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/fullscreen.png"
    PROPERTIES QT_RESOURCE_ALIAS "fullscreen.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/function.png"
    PROPERTIES QT_RESOURCE_ALIAS "function.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/global.png"
    PROPERTIES QT_RESOURCE_ALIAS "global.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/gobo.png"
    PROPERTIES QT_RESOURCE_ALIAS "gobo.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/grid.png"
    PROPERTIES QT_RESOURCE_ALIAS "grid.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/group.png"
    PROPERTIES QT_RESOURCE_ALIAS "group.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/hazer.png"
    PROPERTIES QT_RESOURCE_ALIAS "hazer.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/help.png"
    PROPERTIES QT_RESOURCE_ALIAS "help.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/image.png"
    PROPERTIES QT_RESOURCE_ALIAS "image.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/input.png"
    PROPERTIES QT_RESOURCE_ALIAS "input.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/input_output.png"
    PROPERTIES QT_RESOURCE_ALIAS "input_output.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/intensity.png"
    PROPERTIES QT_RESOURCE_ALIAS "intensity.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/key_bindings.png"
    PROPERTIES QT_RESOURCE_ALIAS "key_bindings.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/knob.png"
    PROPERTIES QT_RESOURCE_ALIAS "knob.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/label.png"
    PROPERTIES QT_RESOURCE_ALIAS "label.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/laser.png"
    PROPERTIES QT_RESOURCE_ALIAS "laser.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/ledbar_beams.png"
    PROPERTIES QT_RESOURCE_ALIAS "ledbar_beams.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/ledbar_pixels.png"
    PROPERTIES QT_RESOURCE_ALIAS "ledbar_pixels.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/liveedit.png"
    PROPERTIES QT_RESOURCE_ALIAS "liveedit.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/liveedit_vc.png"
    PROPERTIES QT_RESOURCE_ALIAS "liveedit_vc.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/lock.png"
    PROPERTIES QT_RESOURCE_ALIAS "lock.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/monitor.png"
    PROPERTIES QT_RESOURCE_ALIAS "monitor.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/movinghead.png"
    PROPERTIES QT_RESOURCE_ALIAS "movinghead.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/operate.png"
    PROPERTIES QT_RESOURCE_ALIAS "operate.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/other.png"
    PROPERTIES QT_RESOURCE_ALIAS "other.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/pan.png"
    PROPERTIES QT_RESOURCE_ALIAS "pan.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/panic.png"
    PROPERTIES QT_RESOURCE_ALIAS "panic.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/player_pause.png"
    PROPERTIES QT_RESOURCE_ALIAS "player_pause.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/player_play.png"
    PROPERTIES QT_RESOURCE_ALIAS "player_play.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/player_stop.png"
    PROPERTIES QT_RESOURCE_ALIAS "player_stop.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/prism.png"
    PROPERTIES QT_RESOURCE_ALIAS "prism.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/qlcplus-fixtureeditor.png"
    PROPERTIES QT_RESOURCE_ALIAS "qlcplus-fixtureeditor.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/qlcplus.png"
    PROPERTIES QT_RESOURCE_ALIAS "qlcplus.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/qt.png"
    PROPERTIES QT_RESOURCE_ALIAS "qt.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/rainbow.png"
    PROPERTIES QT_RESOURCE_ALIAS "rainbow.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/random.png"
    PROPERTIES QT_RESOURCE_ALIAS "random.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/record.png"
    PROPERTIES QT_RESOURCE_ALIAS "record.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/refresh.png"
    PROPERTIES QT_RESOURCE_ALIAS "refresh.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/remap.png"
    PROPERTIES QT_RESOURCE_ALIAS "remap.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/resize.png"
    PROPERTIES QT_RESOURCE_ALIAS "resize.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/rgbmatrix.png"
    PROPERTIES QT_RESOURCE_ALIAS "rgbmatrix.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/rgbpanel.png"
    PROPERTIES QT_RESOURCE_ALIAS "rgbpanel.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/scanner.png"
    PROPERTIES QT_RESOURCE_ALIAS "scanner.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/scene.png"
    PROPERTIES QT_RESOURCE_ALIAS "scene.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/script.png"
    PROPERTIES QT_RESOURCE_ALIAS "script.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/sequence.png"
    PROPERTIES QT_RESOURCE_ALIAS "sequence.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/show.png"
    PROPERTIES QT_RESOURCE_ALIAS "show.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/shutter.png"
    PROPERTIES QT_RESOURCE_ALIAS "shutter.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/slider.png"
    PROPERTIES QT_RESOURCE_ALIAS "slider.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/slidermatrix.png"
    PROPERTIES QT_RESOURCE_ALIAS "slidermatrix.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/smoke.png"
    PROPERTIES QT_RESOURCE_ALIAS "smoke.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/soloframe.png"
    PROPERTIES QT_RESOURCE_ALIAS "soloframe.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/speed.png"
    PROPERTIES QT_RESOURCE_ALIAS "speed.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/square.png"
    PROPERTIES QT_RESOURCE_ALIAS "square.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/star.png"
    PROPERTIES QT_RESOURCE_ALIAS "star.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/strobe.png"
    PROPERTIES QT_RESOURCE_ALIAS "strobe.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/tabview.png"
    PROPERTIES QT_RESOURCE_ALIAS "tabview.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/tilt.png"
    PROPERTIES QT_RESOURCE_ALIAS "tilt.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/uncheck.png"
    PROPERTIES QT_RESOURCE_ALIAS "uncheck.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/undo.png"
    PROPERTIES QT_RESOURCE_ALIAS "undo.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/ungroup.png"
    PROPERTIES QT_RESOURCE_ALIAS "ungroup.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/unlock.png"
    PROPERTIES QT_RESOURCE_ALIAS "unlock.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/up.png"
    PROPERTIES QT_RESOURCE_ALIAS "up.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/video.png"
    PROPERTIES QT_RESOURCE_ALIAS "video.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/virtualconsole.png"
    PROPERTIES QT_RESOURCE_ALIAS "virtualconsole.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/wizard.png"
    PROPERTIES QT_RESOURCE_ALIAS "wizard.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/wizard_256.png"
    PROPERTIES QT_RESOURCE_ALIAS "wizard_256.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/xypad-point-blue.png"
    PROPERTIES QT_RESOURCE_ALIAS "xypad-point-blue.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/xypad-point-yellow.png"
    PROPERTIES QT_RESOURCE_ALIAS "xypad-point-yellow.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/xypad-point.png"
    PROPERTIES QT_RESOURCE_ALIAS "xypad-point.png"
)
set_source_files_properties("../ui/src/../../resources/icons/png/xypad.png"
    PROPERTIES QT_RESOURCE_ALIAS "xypad.png"
)
set(qlcui_resource_files
    "../ui/src/../../resources/icons/png/add_dump.png"
    "../ui/src/../../resources/icons/png/animation.png"
    "../ui/src/../../resources/icons/png/attach.png"
    "../ui/src/../../resources/icons/png/audio.png"
    "../ui/src/../../resources/icons/png/audioinput.png"
    "../ui/src/../../resources/icons/png/autostart.png"
    "../ui/src/../../resources/icons/png/back.png"
    "../ui/src/../../resources/icons/png/beam.png"
    "../ui/src/../../resources/icons/png/blackout.png"
    "../ui/src/../../resources/icons/png/blind.png"
    "../ui/src/../../resources/icons/png/button.png"
    "../ui/src/../../resources/icons/png/buttonmatrix.png"
    "../ui/src/../../resources/icons/png/chaser.png"
    "../ui/src/../../resources/icons/png/check.png"
    "../ui/src/../../resources/icons/png/checkbox_empty.png"
    "../ui/src/../../resources/icons/png/checkbox_full.png"
    "../ui/src/../../resources/icons/png/clock.png"
    "../ui/src/../../resources/icons/png/collection.png"
    "../ui/src/../../resources/icons/png/color.png"
    "../ui/src/../../resources/icons/png/colorwheel.png"
    "../ui/src/../../resources/icons/png/configure.png"
    "../ui/src/../../resources/icons/png/cuelist.png"
    "../ui/src/../../resources/icons/png/current.png"
    "../ui/src/../../resources/icons/png/delete.png"
    "../ui/src/../../resources/icons/png/design.png"
    "../ui/src/../../resources/icons/png/detach.png"
    "../ui/src/../../resources/icons/png/dimmer.png"
    "../ui/src/../../resources/icons/png/diptool.png"
    "../ui/src/../../resources/icons/png/down.png"
    "../ui/src/../../resources/icons/png/ds_border.png"
    "../ui/src/../../resources/icons/png/ds_bottom.png"
    "../ui/src/../../resources/icons/png/ds_off.png"
    "../ui/src/../../resources/icons/png/ds_on.png"
    "../ui/src/../../resources/icons/png/ds_top.png"
    "../ui/src/../../resources/icons/png/edit.png"
    "../ui/src/../../resources/icons/png/edit_add.png"
    "../ui/src/../../resources/icons/png/edit_remove.png"
    "../ui/src/../../resources/icons/png/editclear.png"
    "../ui/src/../../resources/icons/png/editcopy.png"
    "../ui/src/../../resources/icons/png/editcopyall.png"
    "../ui/src/../../resources/icons/png/editcut.png"
    "../ui/src/../../resources/icons/png/editdelete.png"
    "../ui/src/../../resources/icons/png/editpaste.png"
    "../ui/src/../../resources/icons/png/effect.png"
    "../ui/src/../../resources/icons/png/efx.png"
    "../ui/src/../../resources/icons/png/exit.png"
    "../ui/src/../../resources/icons/png/expand.png"
    "../ui/src/../../resources/icons/png/fade.png"
    "../ui/src/../../resources/icons/png/fan.png"
    "../ui/src/../../resources/icons/png/fileclose.png"
    "../ui/src/../../resources/icons/png/fileexport.png"
    "../ui/src/../../resources/icons/png/fileimport.png"
    "../ui/src/../../resources/icons/png/filenew.png"
    "../ui/src/../../resources/icons/png/fileopen.png"
    "../ui/src/../../resources/icons/png/filesave.png"
    "../ui/src/../../resources/icons/png/filesaveas.png"
    "../ui/src/../../resources/icons/png/fixture.png"
    "../ui/src/../../resources/icons/png/flash.png"
    "../ui/src/../../resources/icons/png/flower.png"
    "../ui/src/../../resources/icons/png/folder.png"
    "../ui/src/../../resources/icons/png/fontcolor.png"
    "../ui/src/../../resources/icons/png/fonts.png"
    "../ui/src/../../resources/icons/png/forward.png"
    "../ui/src/../../resources/icons/png/frame.png"
    "../ui/src/../../resources/icons/png/frameraised.png"
    "../ui/src/../../resources/icons/png/framesunken.png"
    "../ui/src/../../resources/icons/png/fullscreen.png"
    "../ui/src/../../resources/icons/png/function.png"
    "../ui/src/../../resources/icons/png/global.png"
    "../ui/src/../../resources/icons/png/gobo.png"
    "../ui/src/../../resources/icons/png/grid.png"
    "../ui/src/../../resources/icons/png/group.png"
    "../ui/src/../../resources/icons/png/hazer.png"
    "../ui/src/../../resources/icons/png/help.png"
    "../ui/src/../../resources/icons/png/image.png"
    "../ui/src/../../resources/icons/png/input.png"
    "../ui/src/../../resources/icons/png/input_output.png"
    "../ui/src/../../resources/icons/png/intensity.png"
    "../ui/src/../../resources/icons/png/key_bindings.png"
    "../ui/src/../../resources/icons/png/knob.png"
    "../ui/src/../../resources/icons/png/label.png"
    "../ui/src/../../resources/icons/png/laser.png"
    "../ui/src/../../resources/icons/png/ledbar_beams.png"
    "../ui/src/../../resources/icons/png/ledbar_pixels.png"
    "../ui/src/../../resources/icons/png/liveedit.png"
    "../ui/src/../../resources/icons/png/liveedit_vc.png"
    "../ui/src/../../resources/icons/png/lock.png"
    "../ui/src/../../resources/icons/png/monitor.png"
    "../ui/src/../../resources/icons/png/movinghead.png"
    "../ui/src/../../resources/icons/png/operate.png"
    "../ui/src/../../resources/icons/png/other.png"
    "../ui/src/../../resources/icons/png/pan.png"
    "../ui/src/../../resources/icons/png/panic.png"
    "../ui/src/../../resources/icons/png/player_pause.png"
    "../ui/src/../../resources/icons/png/player_play.png"
    "../ui/src/../../resources/icons/png/player_stop.png"
    "../ui/src/../../resources/icons/png/prism.png"
    "../ui/src/../../resources/icons/png/qlcplus-fixtureeditor.png"
    "../ui/src/../../resources/icons/png/qlcplus.png"
    "../ui/src/../../resources/icons/png/qt.png"
    "../ui/src/../../resources/icons/png/rainbow.png"
    "../ui/src/../../resources/icons/png/random.png"
    "../ui/src/../../resources/icons/png/record.png"
    "../ui/src/../../resources/icons/png/refresh.png"
    "../ui/src/../../resources/icons/png/remap.png"
    "../ui/src/../../resources/icons/png/resize.png"
    "../ui/src/../../resources/icons/png/rgbmatrix.png"
    "../ui/src/../../resources/icons/png/rgbpanel.png"
    "../ui/src/../../resources/icons/png/scanner.png"
    "../ui/src/../../resources/icons/png/scene.png"
    "../ui/src/../../resources/icons/png/script.png"
    "../ui/src/../../resources/icons/png/sequence.png"
    "../ui/src/../../resources/icons/png/show.png"
    "../ui/src/../../resources/icons/png/shutter.png"
    "../ui/src/../../resources/icons/png/slider.png"
    "../ui/src/../../resources/icons/png/slidermatrix.png"
    "../ui/src/../../resources/icons/png/smoke.png"
    "../ui/src/../../resources/icons/png/soloframe.png"
    "../ui/src/../../resources/icons/png/speed.png"
    "../ui/src/../../resources/icons/png/square.png"
    "../ui/src/../../resources/icons/png/star.png"
    "../ui/src/../../resources/icons/png/strobe.png"
    "../ui/src/../../resources/icons/png/tabview.png"
    "../ui/src/../../resources/icons/png/tilt.png"
    "../ui/src/../../resources/icons/png/uncheck.png"
    "../ui/src/../../resources/icons/png/undo.png"
    "../ui/src/../../resources/icons/png/ungroup.png"
    "../ui/src/../../resources/icons/png/unlock.png"
    "../ui/src/../../resources/icons/png/up.png"
    "../ui/src/../../resources/icons/png/video.png"
    "../ui/src/../../resources/icons/png/virtualconsole.png"
    "../ui/src/../../resources/icons/png/wizard.png"
    "../ui/src/../../resources/icons/png/wizard_256.png"
    "../ui/src/../../resources/icons/png/xypad-point-blue.png"
    "../ui/src/../../resources/icons/png/xypad-point-yellow.png"
    "../ui/src/../../resources/icons/png/xypad-point.png"
    "../ui/src/../../resources/icons/png/xypad.png"
)

if(WIN32)
    target_sources(${module_name} PRIVATE
        fixtureeditor.rc
    )
endif()

if(qmlui OR (QT_VERSION_MAJOR GREATER 5))
    target_link_libraries(${module_name} PRIVATE
        Qt${QT_MAJOR_VERSION}::Qml
    )

    qt_add_resources(${module_name} "qlcui"
        PREFIX
            "/"
        BASE
            "../ui/src"
        FILES
            ${qlcui_resource_files}
    )
else()
    target_link_libraries(${module_name} PRIVATE
        Qt${QT_MAJOR_VERSION}::Script
    )
    qt5_add_resources(qlcplusui_resource_files ../ui/src/qlcui.qrc)
    target_sources(${module_name} PRIVATE
        ${qlcplusui_resource_files}
    )
endif()

install(TARGETS ${module_name}
    DESTINATION ${INSTALLROOT}/${BINDIR}
)

