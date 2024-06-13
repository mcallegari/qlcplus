set(module_name "qlcplusui")

set(TS_FILES
    qlcplus_fi_FI.ts
    qlcplus_fr_FR.ts
    qlcplus_es_ES.ts
    qlcplus_de_DE.ts
    qlcplus_it_IT.ts
    qlcplus_nl_NL.ts
    qlcplus_cz_CZ.ts
    qlcplus_pt_BR.ts
    qlcplus_ca_ES.ts
    qlcplus_ja_JP.ts
)

if(QT_VERSION_MAJOR GREATER 5)
    qt_add_translation(QM_FILES ${TS_FILES})
else()
    qt5_add_translation(QM_FILES ${TS_FILES})
endif()

if((((QT_VERSION_MAJOR LESS 5)) AND (APPLE)))
    if (${PORTAUDIO_2_FOUND})
        target_compile_definitions(${module_name} PUBLIC
            HAS_PORTAUDIO
        )

        target_include_directories(${module_name} PUBLIC
            ${PORTAUDIO_2_INCLUDE_DIRS}
        )

        target_link_libraries(${module_name} PUBLIC
            ${PORTAUDIO_2_LIBRARIES}
        )
    endif()
endif()

add_library(${module_name}
    SHARED
    ../../plugins/interfaces/rdmprotocol.cpp ../../plugins/interfaces/rdmprotocol.h
    aboutbox.cpp aboutbox.h aboutbox.ui
    addchannelsgroup.cpp addchannelsgroup.h addchannelsgroup.ui
    addfixture.cpp addfixture.h addfixture.ui
    addresstool.cpp addresstool.h addresstool.ui
    addrgbpanel.cpp addrgbpanel.h addrgbpanel.ui
    app.cpp app.h
    apputil.cpp apputil.h
    assignhotkey.cpp assignhotkey.h assignhotkey.ui
    audiobar.cpp audiobar.h
    audioeditor.cpp audioeditor.h audioeditor.ui
    audiotriggerwidget.cpp audiotriggerwidget.h
    channelmodifiereditor.cpp channelmodifiereditor.h channelmodifiereditor.ui
    channelmodifiergraphicsview.cpp channelmodifiergraphicsview.h
    channelsselection.cpp channelsselection.h channelsselection.ui
    chasereditor.cpp chasereditor.h chasereditor.ui
    clickandgoslider.cpp clickandgoslider.h
    clickandgowidget.cpp clickandgowidget.h
    collectioneditor.cpp collectioneditor.h collectioneditor.ui
    consolechannel.cpp consolechannel.h
    createfixturegroup.cpp createfixturegroup.h createfixturegroup.ui
    ctkrangeslider.cpp ctkrangeslider.h
    cuestackmodel.cpp cuestackmodel.h
    customfeedbackdialog.cpp customfeedbackdialog.h customfeedbackdialog.ui
    dmxdumpfactory.cpp dmxdumpfactory.h dmxdumpfactory.ui
    efxeditor.cpp efxeditor.h efxeditor.ui
    efxpreviewarea.cpp efxpreviewarea.h
    fixtureconsole.cpp fixtureconsole.h
    fixturegroupeditor.cpp fixturegroupeditor.h fixturegroupeditor.ui
    fixturemanager.cpp fixturemanager.h
    fixtureremap.cpp fixtureremap.h fixtureremap.ui
    fixtureselection.cpp fixtureselection.h fixtureselection.ui
    fixturetreewidget.cpp fixturetreewidget.h
    flowlayout.cpp flowlayout.h
    functionliveeditdialog.cpp functionliveeditdialog.h
    functionmanager.cpp functionmanager.h
    functionselection.cpp functionselection.h functionselection.ui
    functionstreewidget.cpp functionstreewidget.h
    functionwizard.cpp functionwizard.h functionwizard.ui
    grandmasterslider.cpp grandmasterslider.h
    groupsconsole.cpp groupsconsole.h
    inputchanneleditor.cpp inputchanneleditor.h inputchanneleditor.ui
    inputoutputmanager.cpp inputoutputmanager.h
    inputoutputpatcheditor.cpp inputoutputpatcheditor.h inputoutputpatcheditor.ui
    inputprofileeditor.cpp inputprofileeditor.h inputprofileeditor.ui
    inputselectionwidget.cpp inputselectionwidget.h inputselectionwidget.ui
    knobwidget.cpp knobwidget.h
    monitor/monitor.cpp monitor/monitor.h
    monitor/monitorbackgroundselection.cpp monitor/monitorbackgroundselection.h monitor/monitorbackgroundselection.ui
    monitor/monitorfixture.cpp monitor/monitorfixture.h
    monitor/monitorfixtureitem.cpp monitor/monitorfixtureitem.h
    monitor/monitorfixturepropertieseditor.cpp monitor/monitorfixturepropertieseditor.h monitor/monitorfixturepropertieseditor.ui
    monitor/monitorgraphicsview.cpp monitor/monitorgraphicsview.h
    monitor/monitorlayout.cpp monitor/monitorlayout.h
    palettegenerator.cpp palettegenerator.h
    playbackslider.cpp playbackslider.h
    positiontool.cpp positiontool.h positiontool.ui
    rdmmanager.cpp rdmmanager.h rdmmanager.ui
    remapwidget.cpp remapwidget.h
    rgbitem.cpp rgbitem.h
    rgbmatrixeditor.cpp rgbmatrixeditor.h rgbmatrixeditor.ui
    sceneeditor.cpp sceneeditor.h sceneeditor.ui
    scripteditor.cpp scripteditor.h scripteditor.ui
    selectinputchannel.cpp selectinputchannel.h selectinputchannel.ui
    showmanager/audioitem.cpp showmanager/audioitem.h
    showmanager/efxitem.cpp showmanager/efxitem.h
    showmanager/headeritems.cpp showmanager/headeritems.h
    showmanager/multitrackview.cpp showmanager/multitrackview.h
    showmanager/rgbmatrixitem.cpp showmanager/rgbmatrixitem.h
    showmanager/sequenceitem.cpp showmanager/sequenceitem.h
    showmanager/showeditor.cpp showmanager/showeditor.h showmanager/showeditor.ui
    showmanager/showitem.cpp showmanager/showitem.h
    showmanager/showmanager.cpp showmanager/showmanager.h
    showmanager/timingstool.cpp showmanager/timingstool.h
    showmanager/trackitem.cpp showmanager/trackitem.h
    showmanager/videoitem.cpp showmanager/videoitem.h
    simpledesk.cpp simpledesk.h
    simpledeskengine.cpp simpledeskengine.h
    speeddial.cpp speeddial.h
    speeddialwidget.cpp speeddialwidget.h
    universeitemwidget.cpp universeitemwidget.h
    videoeditor.cpp videoeditor.h videoeditor.ui
    videoprovider.cpp videoprovider.h
    virtualconsole/addvcbuttonmatrix.cpp virtualconsole/addvcbuttonmatrix.h virtualconsole/addvcbuttonmatrix.ui
    virtualconsole/addvcslidermatrix.cpp virtualconsole/addvcslidermatrix.h virtualconsole/addvcslidermatrix.ui
    virtualconsole/vcaudiotriggers.cpp virtualconsole/vcaudiotriggers.h
    virtualconsole/vcaudiotriggersproperties.cpp virtualconsole/vcaudiotriggersproperties.h virtualconsole/vcaudiotriggersproperties.ui
    virtualconsole/vcbutton.cpp virtualconsole/vcbutton.h
    virtualconsole/vcbuttonproperties.cpp virtualconsole/vcbuttonproperties.h virtualconsole/vcbuttonproperties.ui
    virtualconsole/vcclock.cpp virtualconsole/vcclock.h
    virtualconsole/vcclockproperties.cpp virtualconsole/vcclockproperties.h virtualconsole/vcclockproperties.ui
    virtualconsole/vccuelist.cpp virtualconsole/vccuelist.h
    virtualconsole/vccuelistproperties.cpp virtualconsole/vccuelistproperties.h virtualconsole/vccuelistproperties.ui
    virtualconsole/vcdockarea.cpp virtualconsole/vcdockarea.h
    virtualconsole/vcframe.cpp virtualconsole/vcframe.h
    virtualconsole/vcframepageshortcut.cpp virtualconsole/vcframepageshortcut.h
    virtualconsole/vcframeproperties.cpp virtualconsole/vcframeproperties.h virtualconsole/vcframeproperties.ui
    virtualconsole/vclabel.cpp virtualconsole/vclabel.h
    virtualconsole/vcmatrix.cpp virtualconsole/vcmatrix.h
    virtualconsole/vcmatrixcontrol.cpp virtualconsole/vcmatrixcontrol.h
    virtualconsole/vcmatrixpresetselection.cpp virtualconsole/vcmatrixpresetselection.h virtualconsole/vcmatrixpresetselection.ui
    virtualconsole/vcmatrixproperties.cpp virtualconsole/vcmatrixproperties.h virtualconsole/vcmatrixproperties.ui
    virtualconsole/vcproperties.cpp virtualconsole/vcproperties.h virtualconsole/vcproperties.ui
    virtualconsole/vcpropertieseditor.cpp virtualconsole/vcpropertieseditor.h
    virtualconsole/vcscrollarea.cpp virtualconsole/vcscrollarea.h
    virtualconsole/vcslider.cpp virtualconsole/vcslider.h
    virtualconsole/vcsliderproperties.cpp virtualconsole/vcsliderproperties.h virtualconsole/vcsliderproperties.ui
    virtualconsole/vcsoloframe.cpp virtualconsole/vcsoloframe.h
    virtualconsole/vcsoloframeproperties.cpp virtualconsole/vcsoloframeproperties.h
    virtualconsole/vcspeeddial.cpp virtualconsole/vcspeeddial.h
    virtualconsole/vcspeeddialfunction.cpp virtualconsole/vcspeeddialfunction.h
    virtualconsole/vcspeeddialpreset.cpp virtualconsole/vcspeeddialpreset.h
    virtualconsole/vcspeeddialproperties.cpp virtualconsole/vcspeeddialproperties.h virtualconsole/vcspeeddialproperties.ui
    virtualconsole/vcwidget.cpp virtualconsole/vcwidget.h
    virtualconsole/vcwidgetproperties.cpp virtualconsole/vcwidgetproperties.h
    virtualconsole/vcwidgetselection.cpp virtualconsole/vcwidgetselection.h virtualconsole/vcwidgetselection.ui
    virtualconsole/vcxypad.cpp virtualconsole/vcxypad.h
    virtualconsole/vcxypadarea.cpp virtualconsole/vcxypadarea.h
    virtualconsole/vcxypadfixture.cpp virtualconsole/vcxypadfixture.h
    virtualconsole/vcxypadfixtureeditor.cpp virtualconsole/vcxypadfixtureeditor.h virtualconsole/vcxypadfixtureeditor.ui
    virtualconsole/vcxypadpreset.cpp virtualconsole/vcxypadpreset.h
    virtualconsole/vcxypadproperties.cpp virtualconsole/vcxypadproperties.h virtualconsole/vcxypadproperties.ui
    virtualconsole/virtualconsole.cpp virtualconsole/virtualconsole.h
    ${QM_FILES}
)

target_include_directories(${module_name} PUBLIC
    ../../engine/audio/src
    ../../engine/src
    ../../plugins/interfaces
    ../../webaccess
    monitor
    showmanager
    virtualconsole
)

target_link_libraries(${module_name} PUBLIC
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Multimedia
    Qt${QT_MAJOR_VERSION}::MultimediaWidgets
    Qt${QT_MAJOR_VERSION}::Widgets
    qlcplusengine
)

# Resources:
set_source_files_properties("../../resources/icons/png/add_dump.png"
    PROPERTIES QT_RESOURCE_ALIAS "add_dump.png"
)
set_source_files_properties("../../resources/icons/png/animation.png"
    PROPERTIES QT_RESOURCE_ALIAS "animation.png"
)
set_source_files_properties("../../resources/icons/png/attach.png"
    PROPERTIES QT_RESOURCE_ALIAS "attach.png"
)
set_source_files_properties("../../resources/icons/png/audio.png"
    PROPERTIES QT_RESOURCE_ALIAS "audio.png"
)
set_source_files_properties("../../resources/icons/png/audioinput.png"
    PROPERTIES QT_RESOURCE_ALIAS "audioinput.png"
)
set_source_files_properties("../../resources/icons/png/autostart.png"
    PROPERTIES QT_RESOURCE_ALIAS "autostart.png"
)
set_source_files_properties("../../resources/icons/png/back.png"
    PROPERTIES QT_RESOURCE_ALIAS "back.png"
)
set_source_files_properties("../../resources/icons/png/beam.png"
    PROPERTIES QT_RESOURCE_ALIAS "beam.png"
)
set_source_files_properties("../../resources/icons/png/blackout.png"
    PROPERTIES QT_RESOURCE_ALIAS "blackout.png"
)
set_source_files_properties("../../resources/icons/png/blind.png"
    PROPERTIES QT_RESOURCE_ALIAS "blind.png"
)
set_source_files_properties("../../resources/icons/png/button.png"
    PROPERTIES QT_RESOURCE_ALIAS "button.png"
)
set_source_files_properties("../../resources/icons/png/buttonmatrix.png"
    PROPERTIES QT_RESOURCE_ALIAS "buttonmatrix.png"
)
set_source_files_properties("../../resources/icons/png/chaser.png"
    PROPERTIES QT_RESOURCE_ALIAS "chaser.png"
)
set_source_files_properties("../../resources/icons/png/check.png"
    PROPERTIES QT_RESOURCE_ALIAS "check.png"
)
set_source_files_properties("../../resources/icons/png/checkbox_empty.png"
    PROPERTIES QT_RESOURCE_ALIAS "checkbox_empty.png"
)
set_source_files_properties("../../resources/icons/png/checkbox_full.png"
    PROPERTIES QT_RESOURCE_ALIAS "checkbox_full.png"
)
set_source_files_properties("../../resources/icons/png/clock.png"
    PROPERTIES QT_RESOURCE_ALIAS "clock.png"
)
set_source_files_properties("../../resources/icons/png/collection.png"
    PROPERTIES QT_RESOURCE_ALIAS "collection.png"
)
set_source_files_properties("../../resources/icons/png/color.png"
    PROPERTIES QT_RESOURCE_ALIAS "color.png"
)
set_source_files_properties("../../resources/icons/png/colorwheel.png"
    PROPERTIES QT_RESOURCE_ALIAS "colorwheel.png"
)
set_source_files_properties("../../resources/icons/png/configure.png"
    PROPERTIES QT_RESOURCE_ALIAS "configure.png"
)
set_source_files_properties("../../resources/icons/png/cuelist.png"
    PROPERTIES QT_RESOURCE_ALIAS "cuelist.png"
)
set_source_files_properties("../../resources/icons/png/current.png"
    PROPERTIES QT_RESOURCE_ALIAS "current.png"
)
set_source_files_properties("../../resources/icons/png/delete.png"
    PROPERTIES QT_RESOURCE_ALIAS "delete.png"
)
set_source_files_properties("../../resources/icons/png/design.png"
    PROPERTIES QT_RESOURCE_ALIAS "design.png"
)
set_source_files_properties("../../resources/icons/png/detach.png"
    PROPERTIES QT_RESOURCE_ALIAS "detach.png"
)
set_source_files_properties("../../resources/icons/png/dimmer.png"
    PROPERTIES QT_RESOURCE_ALIAS "dimmer.png"
)
set_source_files_properties("../../resources/icons/png/diptool.png"
    PROPERTIES QT_RESOURCE_ALIAS "diptool.png"
)
set_source_files_properties("../../resources/icons/png/down.png"
    PROPERTIES QT_RESOURCE_ALIAS "down.png"
)
set_source_files_properties("../../resources/icons/png/ds_border.png"
    PROPERTIES QT_RESOURCE_ALIAS "ds_border.png"
)
set_source_files_properties("../../resources/icons/png/ds_bottom.png"
    PROPERTIES QT_RESOURCE_ALIAS "ds_bottom.png"
)
set_source_files_properties("../../resources/icons/png/ds_off.png"
    PROPERTIES QT_RESOURCE_ALIAS "ds_off.png"
)
set_source_files_properties("../../resources/icons/png/ds_on.png"
    PROPERTIES QT_RESOURCE_ALIAS "ds_on.png"
)
set_source_files_properties("../../resources/icons/png/ds_top.png"
    PROPERTIES QT_RESOURCE_ALIAS "ds_top.png"
)
set_source_files_properties("../../resources/icons/png/edit.png"
    PROPERTIES QT_RESOURCE_ALIAS "edit.png"
)
set_source_files_properties("../../resources/icons/png/edit_add.png"
    PROPERTIES QT_RESOURCE_ALIAS "edit_add.png"
)
set_source_files_properties("../../resources/icons/png/edit_remove.png"
    PROPERTIES QT_RESOURCE_ALIAS "edit_remove.png"
)
set_source_files_properties("../../resources/icons/png/editclear.png"
    PROPERTIES QT_RESOURCE_ALIAS "editclear.png"
)
set_source_files_properties("../../resources/icons/png/editcopy.png"
    PROPERTIES QT_RESOURCE_ALIAS "editcopy.png"
)
set_source_files_properties("../../resources/icons/png/editcopyall.png"
    PROPERTIES QT_RESOURCE_ALIAS "editcopyall.png"
)
set_source_files_properties("../../resources/icons/png/editcut.png"
    PROPERTIES QT_RESOURCE_ALIAS "editcut.png"
)
set_source_files_properties("../../resources/icons/png/editdelete.png"
    PROPERTIES QT_RESOURCE_ALIAS "editdelete.png"
)
set_source_files_properties("../../resources/icons/png/editpaste.png"
    PROPERTIES QT_RESOURCE_ALIAS "editpaste.png"
)
set_source_files_properties("../../resources/icons/png/effect.png"
    PROPERTIES QT_RESOURCE_ALIAS "effect.png"
)
set_source_files_properties("../../resources/icons/png/efx.png"
    PROPERTIES QT_RESOURCE_ALIAS "efx.png"
)
set_source_files_properties("../../resources/icons/png/exit.png"
    PROPERTIES QT_RESOURCE_ALIAS "exit.png"
)
set_source_files_properties("../../resources/icons/png/expand.png"
    PROPERTIES QT_RESOURCE_ALIAS "expand.png"
)
set_source_files_properties("../../resources/icons/png/fade.png"
    PROPERTIES QT_RESOURCE_ALIAS "fade.png"
)
set_source_files_properties("../../resources/icons/png/fan.png"
    PROPERTIES QT_RESOURCE_ALIAS "fan.png"
)
set_source_files_properties("../../resources/icons/png/fileclose.png"
    PROPERTIES QT_RESOURCE_ALIAS "fileclose.png"
)
set_source_files_properties("../../resources/icons/png/fileexport.png"
    PROPERTIES QT_RESOURCE_ALIAS "fileexport.png"
)
set_source_files_properties("../../resources/icons/png/fileimport.png"
    PROPERTIES QT_RESOURCE_ALIAS "fileimport.png"
)
set_source_files_properties("../../resources/icons/png/filenew.png"
    PROPERTIES QT_RESOURCE_ALIAS "filenew.png"
)
set_source_files_properties("../../resources/icons/png/fileopen.png"
    PROPERTIES QT_RESOURCE_ALIAS "fileopen.png"
)
set_source_files_properties("../../resources/icons/png/filesave.png"
    PROPERTIES QT_RESOURCE_ALIAS "filesave.png"
)
set_source_files_properties("../../resources/icons/png/filesaveas.png"
    PROPERTIES QT_RESOURCE_ALIAS "filesaveas.png"
)
set_source_files_properties("../../resources/icons/png/fixture.png"
    PROPERTIES QT_RESOURCE_ALIAS "fixture.png"
)
set_source_files_properties("../../resources/icons/png/flash.png"
    PROPERTIES QT_RESOURCE_ALIAS "flash.png"
)
set_source_files_properties("../../resources/icons/png/flower.png"
    PROPERTIES QT_RESOURCE_ALIAS "flower.png"
)
set_source_files_properties("../../resources/icons/png/folder.png"
    PROPERTIES QT_RESOURCE_ALIAS "folder.png"
)
set_source_files_properties("../../resources/icons/png/fontcolor.png"
    PROPERTIES QT_RESOURCE_ALIAS "fontcolor.png"
)
set_source_files_properties("../../resources/icons/png/fonts.png"
    PROPERTIES QT_RESOURCE_ALIAS "fonts.png"
)
set_source_files_properties("../../resources/icons/png/forward.png"
    PROPERTIES QT_RESOURCE_ALIAS "forward.png"
)
set_source_files_properties("../../resources/icons/png/frame.png"
    PROPERTIES QT_RESOURCE_ALIAS "frame.png"
)
set_source_files_properties("../../resources/icons/png/frameraised.png"
    PROPERTIES QT_RESOURCE_ALIAS "frameraised.png"
)
set_source_files_properties("../../resources/icons/png/framesunken.png"
    PROPERTIES QT_RESOURCE_ALIAS "framesunken.png"
)
set_source_files_properties("../../resources/icons/png/fullscreen.png"
    PROPERTIES QT_RESOURCE_ALIAS "fullscreen.png"
)
set_source_files_properties("../../resources/icons/png/function.png"
    PROPERTIES QT_RESOURCE_ALIAS "function.png"
)
set_source_files_properties("../../resources/icons/png/global.png"
    PROPERTIES QT_RESOURCE_ALIAS "global.png"
)
set_source_files_properties("../../resources/icons/png/gobo.png"
    PROPERTIES QT_RESOURCE_ALIAS "gobo.png"
)
set_source_files_properties("../../resources/icons/png/grid.png"
    PROPERTIES QT_RESOURCE_ALIAS "grid.png"
)
set_source_files_properties("../../resources/icons/png/group.png"
    PROPERTIES QT_RESOURCE_ALIAS "group.png"
)
set_source_files_properties("../../resources/icons/png/hazer.png"
    PROPERTIES QT_RESOURCE_ALIAS "hazer.png"
)
set_source_files_properties("../../resources/icons/png/help.png"
    PROPERTIES QT_RESOURCE_ALIAS "help.png"
)
set_source_files_properties("../../resources/icons/png/image.png"
    PROPERTIES QT_RESOURCE_ALIAS "image.png"
)
set_source_files_properties("../../resources/icons/png/input.png"
    PROPERTIES QT_RESOURCE_ALIAS "input.png"
)
set_source_files_properties("../../resources/icons/png/input_output.png"
    PROPERTIES QT_RESOURCE_ALIAS "input_output.png"
)
set_source_files_properties("../../resources/icons/png/intensity.png"
    PROPERTIES QT_RESOURCE_ALIAS "intensity.png"
)
set_source_files_properties("../../resources/icons/png/key_bindings.png"
    PROPERTIES QT_RESOURCE_ALIAS "key_bindings.png"
)
set_source_files_properties("../../resources/icons/png/knob.png"
    PROPERTIES QT_RESOURCE_ALIAS "knob.png"
)
set_source_files_properties("../../resources/icons/png/label.png"
    PROPERTIES QT_RESOURCE_ALIAS "label.png"
)
set_source_files_properties("../../resources/icons/png/laser.png"
    PROPERTIES QT_RESOURCE_ALIAS "laser.png"
)
set_source_files_properties("../../resources/icons/png/ledbar_beams.png"
    PROPERTIES QT_RESOURCE_ALIAS "ledbar_beams.png"
)
set_source_files_properties("../../resources/icons/png/ledbar_pixels.png"
    PROPERTIES QT_RESOURCE_ALIAS "ledbar_pixels.png"
)
set_source_files_properties("../../resources/icons/png/liveedit.png"
    PROPERTIES QT_RESOURCE_ALIAS "liveedit.png"
)
set_source_files_properties("../../resources/icons/png/liveedit_vc.png"
    PROPERTIES QT_RESOURCE_ALIAS "liveedit_vc.png"
)
set_source_files_properties("../../resources/icons/png/lock.png"
    PROPERTIES QT_RESOURCE_ALIAS "lock.png"
)
set_source_files_properties("../../resources/icons/png/monitor.png"
    PROPERTIES QT_RESOURCE_ALIAS "monitor.png"
)
set_source_files_properties("../../resources/icons/png/movinghead.png"
    PROPERTIES QT_RESOURCE_ALIAS "movinghead.png"
)
set_source_files_properties("../../resources/icons/png/operate.png"
    PROPERTIES QT_RESOURCE_ALIAS "operate.png"
)
set_source_files_properties("../../resources/icons/png/other.png"
    PROPERTIES QT_RESOURCE_ALIAS "other.png"
)
set_source_files_properties("../../resources/icons/png/pan.png"
    PROPERTIES QT_RESOURCE_ALIAS "pan.png"
)
set_source_files_properties("../../resources/icons/png/panic.png"
    PROPERTIES QT_RESOURCE_ALIAS "panic.png"
)
set_source_files_properties("../../resources/icons/png/player_pause.png"
    PROPERTIES QT_RESOURCE_ALIAS "player_pause.png"
)
set_source_files_properties("../../resources/icons/png/player_play.png"
    PROPERTIES QT_RESOURCE_ALIAS "player_play.png"
)
set_source_files_properties("../../resources/icons/png/player_stop.png"
    PROPERTIES QT_RESOURCE_ALIAS "player_stop.png"
)
set_source_files_properties("../../resources/icons/png/prism.png"
    PROPERTIES QT_RESOURCE_ALIAS "prism.png"
)
set_source_files_properties("../../resources/icons/png/qlcplus-fixtureeditor.png"
    PROPERTIES QT_RESOURCE_ALIAS "qlcplus-fixtureeditor.png"
)
set_source_files_properties("../../resources/icons/png/qlcplus.png"
    PROPERTIES QT_RESOURCE_ALIAS "qlcplus.png"
)
set_source_files_properties("../../resources/icons/png/qt.png"
    PROPERTIES QT_RESOURCE_ALIAS "qt.png"
)
set_source_files_properties("../../resources/icons/png/rainbow.png"
    PROPERTIES QT_RESOURCE_ALIAS "rainbow.png"
)
set_source_files_properties("../../resources/icons/png/random.png"
    PROPERTIES QT_RESOURCE_ALIAS "random.png"
)
set_source_files_properties("../../resources/icons/png/record.png"
    PROPERTIES QT_RESOURCE_ALIAS "record.png"
)
set_source_files_properties("../../resources/icons/png/refresh.png"
    PROPERTIES QT_RESOURCE_ALIAS "refresh.png"
)
set_source_files_properties("../../resources/icons/png/remap.png"
    PROPERTIES QT_RESOURCE_ALIAS "remap.png"
)
set_source_files_properties("../../resources/icons/png/resize.png"
    PROPERTIES QT_RESOURCE_ALIAS "resize.png"
)
set_source_files_properties("../../resources/icons/png/rgbmatrix.png"
    PROPERTIES QT_RESOURCE_ALIAS "rgbmatrix.png"
)
set_source_files_properties("../../resources/icons/png/rgbpanel.png"
    PROPERTIES QT_RESOURCE_ALIAS "rgbpanel.png"
)
set_source_files_properties("../../resources/icons/png/scanner.png"
    PROPERTIES QT_RESOURCE_ALIAS "scanner.png"
)
set_source_files_properties("../../resources/icons/png/scene.png"
    PROPERTIES QT_RESOURCE_ALIAS "scene.png"
)
set_source_files_properties("../../resources/icons/png/script.png"
    PROPERTIES QT_RESOURCE_ALIAS "script.png"
)
set_source_files_properties("../../resources/icons/png/sequence.png"
    PROPERTIES QT_RESOURCE_ALIAS "sequence.png"
)
set_source_files_properties("../../resources/icons/png/show.png"
    PROPERTIES QT_RESOURCE_ALIAS "show.png"
)
set_source_files_properties("../../resources/icons/png/shutter.png"
    PROPERTIES QT_RESOURCE_ALIAS "shutter.png"
)
set_source_files_properties("../../resources/icons/png/slider.png"
    PROPERTIES QT_RESOURCE_ALIAS "slider.png"
)
set_source_files_properties("../../resources/icons/png/slidermatrix.png"
    PROPERTIES QT_RESOURCE_ALIAS "slidermatrix.png"
)
set_source_files_properties("../../resources/icons/png/smoke.png"
    PROPERTIES QT_RESOURCE_ALIAS "smoke.png"
)
set_source_files_properties("../../resources/icons/png/soloframe.png"
    PROPERTIES QT_RESOURCE_ALIAS "soloframe.png"
)
set_source_files_properties("../../resources/icons/png/speed.png"
    PROPERTIES QT_RESOURCE_ALIAS "speed.png"
)
set_source_files_properties("../../resources/icons/png/square.png"
    PROPERTIES QT_RESOURCE_ALIAS "square.png"
)
set_source_files_properties("../../resources/icons/png/star.png"
    PROPERTIES QT_RESOURCE_ALIAS "star.png"
)
set_source_files_properties("../../resources/icons/png/strobe.png"
    PROPERTIES QT_RESOURCE_ALIAS "strobe.png"
)
set_source_files_properties("../../resources/icons/png/tabview.png"
    PROPERTIES QT_RESOURCE_ALIAS "tabview.png"
)
set_source_files_properties("../../resources/icons/png/tilt.png"
    PROPERTIES QT_RESOURCE_ALIAS "tilt.png"
)
set_source_files_properties("../../resources/icons/png/uncheck.png"
    PROPERTIES QT_RESOURCE_ALIAS "uncheck.png"
)
set_source_files_properties("../../resources/icons/png/undo.png"
    PROPERTIES QT_RESOURCE_ALIAS "undo.png"
)
set_source_files_properties("../../resources/icons/png/ungroup.png"
    PROPERTIES QT_RESOURCE_ALIAS "ungroup.png"
)
set_source_files_properties("../../resources/icons/png/unlock.png"
    PROPERTIES QT_RESOURCE_ALIAS "unlock.png"
)
set_source_files_properties("../../resources/icons/png/up.png"
    PROPERTIES QT_RESOURCE_ALIAS "up.png"
)
set_source_files_properties("../../resources/icons/png/video.png"
    PROPERTIES QT_RESOURCE_ALIAS "video.png"
)
set_source_files_properties("../../resources/icons/png/virtualconsole.png"
    PROPERTIES QT_RESOURCE_ALIAS "virtualconsole.png"
)
set_source_files_properties("../../resources/icons/png/wizard.png"
    PROPERTIES QT_RESOURCE_ALIAS "wizard.png"
)
set_source_files_properties("../../resources/icons/png/wizard_256.png"
    PROPERTIES QT_RESOURCE_ALIAS "wizard_256.png"
)
set_source_files_properties("../../resources/icons/png/xypad-point-blue.png"
    PROPERTIES QT_RESOURCE_ALIAS "xypad-point-blue.png"
)
set_source_files_properties("../../resources/icons/png/xypad-point-yellow.png"
    PROPERTIES QT_RESOURCE_ALIAS "xypad-point-yellow.png"
)
set_source_files_properties("../../resources/icons/png/xypad-point.png"
    PROPERTIES QT_RESOURCE_ALIAS "xypad-point.png"
)
set_source_files_properties("../../resources/icons/png/xypad.png"
    PROPERTIES QT_RESOURCE_ALIAS "xypad.png"
)
set(qlcui_resource_files
    "../../resources/icons/png/add_dump.png"
    "../../resources/icons/png/animation.png"
    "../../resources/icons/png/attach.png"
    "../../resources/icons/png/audio.png"
    "../../resources/icons/png/audioinput.png"
    "../../resources/icons/png/autostart.png"
    "../../resources/icons/png/back.png"
    "../../resources/icons/png/beam.png"
    "../../resources/icons/png/blackout.png"
    "../../resources/icons/png/blind.png"
    "../../resources/icons/png/button.png"
    "../../resources/icons/png/buttonmatrix.png"
    "../../resources/icons/png/chaser.png"
    "../../resources/icons/png/check.png"
    "../../resources/icons/png/checkbox_empty.png"
    "../../resources/icons/png/checkbox_full.png"
    "../../resources/icons/png/clock.png"
    "../../resources/icons/png/collection.png"
    "../../resources/icons/png/color.png"
    "../../resources/icons/png/colorwheel.png"
    "../../resources/icons/png/configure.png"
    "../../resources/icons/png/cuelist.png"
    "../../resources/icons/png/current.png"
    "../../resources/icons/png/delete.png"
    "../../resources/icons/png/design.png"
    "../../resources/icons/png/detach.png"
    "../../resources/icons/png/dimmer.png"
    "../../resources/icons/png/diptool.png"
    "../../resources/icons/png/down.png"
    "../../resources/icons/png/ds_border.png"
    "../../resources/icons/png/ds_bottom.png"
    "../../resources/icons/png/ds_off.png"
    "../../resources/icons/png/ds_on.png"
    "../../resources/icons/png/ds_top.png"
    "../../resources/icons/png/edit.png"
    "../../resources/icons/png/edit_add.png"
    "../../resources/icons/png/edit_remove.png"
    "../../resources/icons/png/editclear.png"
    "../../resources/icons/png/editcopy.png"
    "../../resources/icons/png/editcopyall.png"
    "../../resources/icons/png/editcut.png"
    "../../resources/icons/png/editdelete.png"
    "../../resources/icons/png/editpaste.png"
    "../../resources/icons/png/effect.png"
    "../../resources/icons/png/efx.png"
    "../../resources/icons/png/exit.png"
    "../../resources/icons/png/expand.png"
    "../../resources/icons/png/fade.png"
    "../../resources/icons/png/fan.png"
    "../../resources/icons/png/fileclose.png"
    "../../resources/icons/png/fileexport.png"
    "../../resources/icons/png/fileimport.png"
    "../../resources/icons/png/filenew.png"
    "../../resources/icons/png/fileopen.png"
    "../../resources/icons/png/filesave.png"
    "../../resources/icons/png/filesaveas.png"
    "../../resources/icons/png/fixture.png"
    "../../resources/icons/png/flash.png"
    "../../resources/icons/png/flower.png"
    "../../resources/icons/png/folder.png"
    "../../resources/icons/png/fontcolor.png"
    "../../resources/icons/png/fonts.png"
    "../../resources/icons/png/forward.png"
    "../../resources/icons/png/frame.png"
    "../../resources/icons/png/frameraised.png"
    "../../resources/icons/png/framesunken.png"
    "../../resources/icons/png/fullscreen.png"
    "../../resources/icons/png/function.png"
    "../../resources/icons/png/global.png"
    "../../resources/icons/png/gobo.png"
    "../../resources/icons/png/grid.png"
    "../../resources/icons/png/group.png"
    "../../resources/icons/png/hazer.png"
    "../../resources/icons/png/help.png"
    "../../resources/icons/png/image.png"
    "../../resources/icons/png/input.png"
    "../../resources/icons/png/input_output.png"
    "../../resources/icons/png/intensity.png"
    "../../resources/icons/png/key_bindings.png"
    "../../resources/icons/png/knob.png"
    "../../resources/icons/png/label.png"
    "../../resources/icons/png/laser.png"
    "../../resources/icons/png/ledbar_beams.png"
    "../../resources/icons/png/ledbar_pixels.png"
    "../../resources/icons/png/liveedit.png"
    "../../resources/icons/png/liveedit_vc.png"
    "../../resources/icons/png/lock.png"
    "../../resources/icons/png/monitor.png"
    "../../resources/icons/png/movinghead.png"
    "../../resources/icons/png/operate.png"
    "../../resources/icons/png/other.png"
    "../../resources/icons/png/pan.png"
    "../../resources/icons/png/panic.png"
    "../../resources/icons/png/player_pause.png"
    "../../resources/icons/png/player_play.png"
    "../../resources/icons/png/player_stop.png"
    "../../resources/icons/png/prism.png"
    "../../resources/icons/png/qlcplus-fixtureeditor.png"
    "../../resources/icons/png/qlcplus.png"
    "../../resources/icons/png/qt.png"
    "../../resources/icons/png/rainbow.png"
    "../../resources/icons/png/random.png"
    "../../resources/icons/png/record.png"
    "../../resources/icons/png/refresh.png"
    "../../resources/icons/png/remap.png"
    "../../resources/icons/png/resize.png"
    "../../resources/icons/png/rgbmatrix.png"
    "../../resources/icons/png/rgbpanel.png"
    "../../resources/icons/png/scanner.png"
    "../../resources/icons/png/scene.png"
    "../../resources/icons/png/script.png"
    "../../resources/icons/png/sequence.png"
    "../../resources/icons/png/show.png"
    "../../resources/icons/png/shutter.png"
    "../../resources/icons/png/slider.png"
    "../../resources/icons/png/slidermatrix.png"
    "../../resources/icons/png/smoke.png"
    "../../resources/icons/png/soloframe.png"
    "../../resources/icons/png/speed.png"
    "../../resources/icons/png/square.png"
    "../../resources/icons/png/star.png"
    "../../resources/icons/png/strobe.png"
    "../../resources/icons/png/tabview.png"
    "../../resources/icons/png/tilt.png"
    "../../resources/icons/png/uncheck.png"
    "../../resources/icons/png/undo.png"
    "../../resources/icons/png/ungroup.png"
    "../../resources/icons/png/unlock.png"
    "../../resources/icons/png/up.png"
    "../../resources/icons/png/video.png"
    "../../resources/icons/png/virtualconsole.png"
    "../../resources/icons/png/wizard.png"
    "../../resources/icons/png/wizard_256.png"
    "../../resources/icons/png/xypad-point-blue.png"
    "../../resources/icons/png/xypad-point-yellow.png"
    "../../resources/icons/png/xypad-point.png"
    "../../resources/icons/png/xypad.png"
)

if(QT_VERSION_MAJOR GREATER 5)
    target_link_libraries(${module_name} PUBLIC
        Qt${QT_MAJOR_VERSION}::Qml
    )

    qt_add_resources(${module_name} "qlcui"
        PREFIX
            "/"
        FILES
            ${qlcui_resource_files}
    )
else()
    target_link_libraries(${module_name} PUBLIC
        Qt${QT_MAJOR_VERSION}::Script
    )

    qt5_add_resources(qlcplusui_resource_files "qlcui.qrc")
    target_sources(${module_name} PRIVATE
        ${qlcplusui_resource_files}
    )
endif()

if(WIN32)
    target_include_directories(${module_name} PUBLIC
        ../../hotplugmonitor/src
    )
endif()

install(TARGETS ${module_name}
    LIBRARY DESTINATION ${INSTALLROOT}/${LIBSDIR}
    RUNTIME DESTINATION ${INSTALLROOT}/${LIBSDIR}
)
