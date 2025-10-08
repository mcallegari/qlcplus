set(module_name "qlcplusui")

include(../../variables.cmake)

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

# Add resources and RGB Script deps
if(QT_VERSION_MAJOR GREATER 5)
    target_link_libraries(${module_name} PUBLIC
        Qt${QT_MAJOR_VERSION}::Qml)

    qt_add_resources(qlcplusui_resource_files "qlcui.qrc")
else()
    target_link_libraries(${module_name} PUBLIC
        Qt${QT_MAJOR_VERSION}::Script)

    qt5_add_resources(qlcplusui_resource_files "qlcui.qrc")
endif()
target_sources(${module_name} PRIVATE
    ${qlcplusui_resource_files})

if(WIN32)
    target_include_directories(${module_name} PUBLIC
        ../../hotplugmonitor/src)
endif()

install(TARGETS ${module_name}
    LIBRARY DESTINATION ${INSTALLROOT}/${LIBSDIR}
    RUNTIME DESTINATION ${INSTALLROOT}/${LIBSDIR}
)
