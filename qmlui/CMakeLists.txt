set(module_name "qlcplus-qml")

set(TS_FILES
    qlcplus_ca_ES.ts
    qlcplus_de_DE.ts
    qlcplus_es_ES.ts
    qlcplus_fr_FR.ts
    qlcplus_it_IT.ts
    qlcplus_ja_JP.ts
    qlcplus_nl_NL.ts
    qlcplus_pl_PL.ts
    qlcplus_ru_RU.ts
    qlcplus_uk_UA.ts
)

if(QT_VERSION_MAJOR GREATER 5)
    qt_add_translation(QM_FILES ${TS_FILES})
else()
    qt5_add_translation(QM_FILES ${TS_FILES})
endif()

add_executable(${module_name} WIN32 MACOSX_BUNDLE
    app.cpp app.h
    audioeditor.cpp audioeditor.h
    chasereditor.cpp chasereditor.h
    collectioneditor.cpp collectioneditor.h
    colorfilters.cpp colorfilters.h
    contextmanager.cpp contextmanager.h
    efxeditor.cpp efxeditor.h
    fixturebrowser.cpp fixturebrowser.h
    fixtureeditor/channeledit.cpp fixtureeditor/channeledit.h
    fixtureeditor/editorview.cpp fixtureeditor/editorview.h
    fixtureeditor/fixtureeditor.cpp fixtureeditor/fixtureeditor.h
    fixtureeditor/modeedit.cpp fixtureeditor/modeedit.h
    fixtureeditor/physicaledit.cpp fixtureeditor/physicaledit.h
    fixturegroupeditor.cpp fixturegroupeditor.h
    fixturemanager.cpp fixturemanager.h
    fixtureutils.cpp fixtureutils.h
    folderbrowser.cpp folderbrowser.h
    functioneditor.cpp functioneditor.h
    functionmanager.cpp functionmanager.h
    importmanager.cpp importmanager.h
    inputoutputmanager.cpp inputoutputmanager.h
    listmodel.cpp listmodel.h
    main.cpp
    mainview2d.cpp mainview2d.h
    mainview3d.cpp mainview3d.h
    mainviewdmx.cpp mainviewdmx.h
    modelselector.cpp modelselector.h
    palettemanager.cpp palettemanager.h
    previewcontext.cpp previewcontext.h
    rgbmatrixeditor.cpp rgbmatrixeditor.h
    sceneeditor.cpp sceneeditor.h
    scripteditor.cpp scripteditor.h
    showmanager.cpp showmanager.h
    simpledesk.cpp simpledesk.h
    tardis/networkmanager.cpp tardis/networkmanager.h
    tardis/networkpacketizer.cpp tardis/networkpacketizer.h
    tardis/simplecrypt.cpp tardis/simplecrypt.h
    tardis/tardis.cpp tardis/tardis.h
    treemodel.cpp treemodel.h
    treemodelitem.cpp treemodelitem.h
    uimanager.cpp uimanager.h
    videoeditor.cpp videoeditor.h
    videoprovider.cpp videoprovider.h
    virtualconsole/vcbutton.cpp virtualconsole/vcbutton.h
    virtualconsole/vcclock.cpp virtualconsole/vcclock.h
    virtualconsole/vccuelist.cpp virtualconsole/vccuelist.h
    virtualconsole/vcframe.cpp virtualconsole/vcframe.h
    virtualconsole/vclabel.cpp virtualconsole/vclabel.h
    virtualconsole/vcpage.cpp virtualconsole/vcpage.h
    virtualconsole/vcslider.cpp virtualconsole/vcslider.h
    virtualconsole/vcsoloframe.cpp virtualconsole/vcsoloframe.h
    virtualconsole/vcwidget.cpp virtualconsole/vcwidget.h
    virtualconsole/virtualconsole.cpp virtualconsole/virtualconsole.h
    ${QM_FILES}
)

if(WIN32)
    target_sources(${module_name} PRIVATE
        qmlui.rc
    )
endif()

if(QT_VERSION_MAJOR GREATER 5)
    qt_add_qml_module(${module_name}
        URI qlcplus_qml
        VERSION ${PROJECT_VERSION}
        QML_FILES
            qml/ActionsMenu.qml
            qml/BeatGeneratorsPanel.qml
            qml/ChannelToolLoader.qml
            qml/ChaserStepDelegate.qml
            qml/ChaserWidget.qml
            qml/ColorTool.qml
            qml/ColorToolBasic.qml
            qml/ColorToolFilters.qml
            qml/ColorToolFull.qml
            qml/ColorToolPrimary.qml
            qml/ContextMenuEntry.qml
            qml/CustomCheckBox.qml
            qml/CustomComboBox.qml
            qml/CustomScrollBar.qml
            qml/CustomSlider.qml
            qml/CustomSpinBox.qml
            qml/CustomDoubleSpinBox.qml
            qml/CustomTextEdit.qml
            qml/CustomTextInput.qml
            qml/DayTimeTool.qml
            qml/DMXAddressTool.qml
            qml/DMXAddressWidget.qml
            qml/DMXPercentageButton.qml
            qml/ExternalControls.qml
            qml/ExternalControlDelegate.qml
            qml/FixtureConsole.qml
            qml/FixtureDelegate.qml
            qml/FunctionDelegate.qml
            qml/GenericButton.qml
            qml/GenericMultiDragItem.qml
            qml/FontAwesomeVariables.qml
            qml/IconButton.qml
            qml/IconPopupButton.qml
            qml/IconTextEntry.qml
            qml/InputChannelDelegate.qml
            qml/KeyboardSequenceDelegate.qml
            qml/KeyPad.qml
            qml/MainView.qml
            qml/MenuBarEntry.qml
            qml/MultiColorBox.qml
            qml/PaletteFanningBox.qml
            qml/QLCPlusFader.qml
            qml/QLCPlusKnob.qml
            qml/RobotoText.qml
            qml/SectionBox.qml
            qml/SidePanel.qml
            qml/SimpleDesk.qml
            qml/SingleAxisTool.qml
            qml/TimeEditTool.qml
            qml/TreeNodeDelegate.qml
            qml/UISettings.qml
            qml/UsageList.qml
            qml/WidgetDelegate.qml
            qml/WindowLoader.qml
            qml/ZoomItem.qml
            qml/popup/CustomPopupDialog.qml
            qml/popup/PopupAbout.qml
            qml/popup/PopupChannelWizard.qml
            qml/popup/PopupCreatePalette.qml
            qml/popup/PopupDisclaimer.qml
            qml/popup/PopupFolderBrowser.qml
            qml/popup/PopupImportProject.qml
            qml/popup/PopupPINRequest.qml
            qml/popup/PopupPINSetup.qml
            qml/popup/PopupRenameItems.qml
            qml/popup/PopupDMXDump.qml
            qml/popup/PopupMonitor.qml
            qml/popup/PopupNetworkClient.qml
            qml/popup/PopupNetworkConnect.qml
            qml/popup/PopupNetworkServer.qml
            qml/popup/PopupManualInputSource.qml
            qml/fixturesfunctions/FixturesAndFunctions.qml
            qml/fixturesfunctions/RightPanel.qml
            qml/fixturesfunctions/LeftPanel.qml
            qml/fixturesfunctions/BottomPanel.qml
            qml/fixturesfunctions/IntensityTool.qml
            qml/fixturesfunctions/PositionTool.qml
            qml/fixturesfunctions/PresetsTool.qml
            qml/fixturesfunctions/BeamTool.qml
            qml/fixturesfunctions/PresetCapabilityItem.qml
            qml/fixturesfunctions/FixtureChannelDelegate.qml
            qml/fixturesfunctions/FixtureHeadDelegate.qml
            qml/fixturesfunctions/FixtureBrowser.qml
            qml/fixturesfunctions/FixtureProperties.qml
            qml/fixturesfunctions/RGBPanelProperties.qml
            qml/fixturesfunctions/FixtureDragItem.qml
            qml/fixturesfunctions/FixtureBrowserDelegate.qml
            qml/fixturesfunctions/FixtureGroupEditor.qml
            qml/fixturesfunctions/FixtureGroupManager.qml
            qml/fixturesfunctions/FixtureNodeDelegate.qml
            qml/fixturesfunctions/FixtureSummary.qml
            qml/fixturesfunctions/GridEditor.qml
            qml/fixturesfunctions/UniverseGridView.qml
            qml/fixturesfunctions/UniverseSummary.qml
            qml/fixturesfunctions/DMXView.qml
            qml/fixturesfunctions/FixtureDMXItem.qml
            qml/fixturesfunctions/2DView.qml
            qml/fixturesfunctions/SettingsView2D.qml
            qml/fixturesfunctions/SettingsViewDMX.qml
            qml/fixturesfunctions/Fixture2DItem.qml
            qml/fixturesfunctions/ShutterAnimator.qml
            qml/fixturesfunctions/FunctionManager.qml
            qml/fixturesfunctions/AddFunctionMenu.qml
            qml/fixturesfunctions/EditorTopBar.qml
            qml/fixturesfunctions/AudioEditor.qml
            qml/fixturesfunctions/VideoEditor.qml
            qml/fixturesfunctions/VideoContext.qml
            qml/fixturesfunctions/CollectionEditor.qml
            qml/fixturesfunctions/CollectionFunctionDelegate.qml
            qml/fixturesfunctions/SceneEditor.qml
            qml/fixturesfunctions/SceneFixtureConsole.qml
            qml/fixturesfunctions/ChaserEditor.qml
            qml/fixturesfunctions/SequenceEditor.qml
            qml/fixturesfunctions/RGBMatrixEditor.qml
            qml/fixturesfunctions/RGBMatrixPreview.qml
            qml/fixturesfunctions/EFXEditor.qml
            qml/fixturesfunctions/EFXPreview.qml
            qml/fixturesfunctions/ScriptEditor.qml
            qml/fixturesfunctions/PaletteManager.qml
            qml/fixturesfunctions/3DView/3DView.qml
            qml/fixturesfunctions/3DView/3DViewUnsupported.qml
            qml/fixturesfunctions/3DView/SettingsView3D.qml
            qml/fixturesfunctions/3DView/DeferredRenderer.qml
            qml/fixturesfunctions/3DView/Fixture3DItem.qml
            qml/fixturesfunctions/3DView/LightEntity.qml
            qml/fixturesfunctions/3DView/MultiBeams3DItem.qml
            qml/fixturesfunctions/3DView/PixelBar3DItem.qml
            qml/fixturesfunctions/3DView/Generic3DItem.qml
            qml/fixturesfunctions/3DView/DepthTarget.qml
            qml/fixturesfunctions/3DView/GBuffer.qml
            qml/fixturesfunctions/3DView/FrameTarget.qml
            qml/fixturesfunctions/3DView/SpotlightConeEntity.qml
            qml/fixturesfunctions/3DView/SpotlightShadingEffect.qml
            qml/fixturesfunctions/3DView/SpotlightScatteringEffect.qml
            qml/fixturesfunctions/3DView/LightPassEffect.qml
            qml/fixturesfunctions/3DView/GeometryPassEffect.qml
            qml/fixturesfunctions/3DView/GammaCorrectEffect.qml
            qml/fixturesfunctions/3DView/BlitEffect.qml
            qml/fixturesfunctions/3DView/FXAAEffect.qml
            qml/fixturesfunctions/3DView/GrabBrightEffect.qml
            qml/fixturesfunctions/3DView/DownsampleEffect.qml
            qml/fixturesfunctions/3DView/UpsampleEffect.qml
            qml/fixturesfunctions/3DView/OutputFrontDepthEffect.qml
            qml/fixturesfunctions/3DView/FillGBufferFilter.qml
            qml/fixturesfunctions/3DView/DirectionalLightFilter.qml
            qml/fixturesfunctions/3DView/SpotlightScatteringFilter.qml
            qml/fixturesfunctions/3DView/SpotlightShadingFilter.qml
            qml/fixturesfunctions/3DView/RenderSelectionBoxesFilter.qml
            qml/fixturesfunctions/3DView/OutputFrontDepthFilter.qml
            qml/fixturesfunctions/3DView/RenderShadowMapFilter.qml
            qml/fixturesfunctions/3DView/GrabBrightFilter.qml
            qml/fixturesfunctions/3DView/DownsampleFilter.qml
            qml/fixturesfunctions/3DView/UpsampleFilter.qml
            qml/fixturesfunctions/3DView/GammaCorrectFilter.qml
            qml/fixturesfunctions/3DView/BlitFilter.qml
            qml/fixturesfunctions/3DView/FXAAFilter.qml
            qml/fixturesfunctions/3DView/SceneEntity.qml
            qml/fixturesfunctions/3DView/ScreenQuadEntity.qml
            qml/fixturesfunctions/3DView/ScreenQuadGammaCorrectEntity.qml
            qml/fixturesfunctions/3DView/GenericScreenQuadEntity.qml
            qml/fixturesfunctions/3DView/SelectionEntity.qml
            qml/fixturesfunctions/3DView/SelectionGeometry.qml
            qml/fixturesfunctions/3DView/StageSimple.qml
            qml/fixturesfunctions/3DView/StageBox.qml
            qml/fixturesfunctions/3DView/StageRock.qml
            qml/fixturesfunctions/3DView/StageTheatre.qml
            qml/inputoutput/InputOutputManager.qml
            qml/inputoutput/IOLeftPanel.qml
            qml/inputoutput/IORightPanel.qml
            qml/inputoutput/UniverseIOItem.qml
            qml/inputoutput/AudioIOItem.qml
            qml/inputoutput/AudioCardsList.qml
            qml/inputoutput/AudioDeviceItem.qml
            qml/inputoutput/PluginsList.qml
            qml/inputoutput/ProfilesList.qml
            qml/inputoutput/PluginDragItem.qml
            qml/inputoutput/OutputPatchItem.qml
            qml/inputoutput/InputPatchItem.qml
            qml/inputoutput/PatchWireBox.qml
            qml/virtualconsole/VirtualConsole.qml
            qml/virtualconsole/VCRightPanel.qml
            qml/virtualconsole/VCPageArea.qml
            qml/virtualconsole/VCPageProperties.qml
            qml/virtualconsole/WidgetsList.qml
            qml/virtualconsole/WidgetDragItem.qml
            qml/virtualconsole/VCWidgetItem.qml
            qml/virtualconsole/VCWidgetProperties.qml
            qml/virtualconsole/VCFrameItem.qml
            qml/virtualconsole/VCFrameProperties.qml
            qml/virtualconsole/VCButtonItem.qml
            qml/virtualconsole/VCButtonProperties.qml
            qml/virtualconsole/VCLabelItem.qml
            qml/virtualconsole/VCSliderItem.qml
            qml/virtualconsole/VCSliderProperties.qml
            qml/virtualconsole/VCClockItem.qml
            qml/virtualconsole/VCClockProperties.qml
            qml/virtualconsole/VCCueListItem.qml
            qml/virtualconsole/VCCueListProperties.qml
            qml/showmanager/ShowManager.qml
            qml/showmanager/TrackDelegate.qml
            qml/showmanager/ShowItem.qml
            qml/showmanager/HeaderAndCursor.qml
            qml/fixtureeditor/FixtureEditor.qml
            qml/fixtureeditor/EditorView.qml
            qml/fixtureeditor/PhysicalProperties.qml
            qml/fixtureeditor/ChannelEditor.qml
            qml/fixtureeditor/ModeEditor.qml
            js/CanvasDrawFunctions.js
            js/FixtureDrag.js
            js/GenericHelpers.js
            js/Math3DView.js
            js/TimeUtils.js
        RESOURCES
            qml/qmldir
            qml/fixturesfunctions/3DView/shaders/downsample.frag
            qml/fixturesfunctions/3DView/shaders/upsample.frag
            qml/fixturesfunctions/3DView/shaders/grab_bright.frag
            qml/fixturesfunctions/3DView/shaders/directional.frag
            qml/fixturesfunctions/3DView/shaders/gamma_correct.frag
            qml/fixturesfunctions/3DView/shaders/blit.frag
            qml/fixturesfunctions/3DView/shaders/fxaa.frag
            qml/fixturesfunctions/3DView/shaders/fullscreen.vert
            qml/fixturesfunctions/3DView/shaders/spotlight_shading.frag
            qml/fixturesfunctions/3DView/shaders/geo.frag
            qml/fixturesfunctions/3DView/shaders/geo.vert
            qml/fixturesfunctions/3DView/shaders/spotlight_scattering.frag
            qml/fixturesfunctions/3DView/shaders/spotlight.vert
            qml/fixturesfunctions/3DView/shaders/output_front_depth.frag
            qml/fixturesfunctions/3DView/shaders/output_depth.frag
            qml/fixturesfunctions/3DView/shaders/output_depth.vert
        NO_RESOURCE_TARGET_PATH
    )
endif()

set_source_files_properties("js/CanvasDrawFunctions.js"
    PROPERTIES QT_RESOURCE_ALIAS "CanvasDrawFunctions.js"
)
set_source_files_properties("js/FixtureDrag.js"
    PROPERTIES QT_RESOURCE_ALIAS "FixtureDrag.js"
)
set_source_files_properties("js/GenericHelpers.js"
    PROPERTIES QT_RESOURCE_ALIAS "GenericHelpers.js"
)
set_source_files_properties("js/Math3DView.js"
    PROPERTIES QT_RESOURCE_ALIAS "Math3DView.js"
)
set_source_files_properties("js/TimeUtils.js"
    PROPERTIES QT_RESOURCE_ALIAS "TimeUtils.js"
)
set_source_files_properties("qml/ActionsMenu.qml"
    PROPERTIES QT_RESOURCE_ALIAS "ActionsMenu.qml"
)
set_source_files_properties("qml/BeatGeneratorsPanel.qml"
    PROPERTIES QT_RESOURCE_ALIAS "BeatGeneratorsPanel.qml"
)
set_source_files_properties("qml/ChannelToolLoader.qml"
    PROPERTIES QT_RESOURCE_ALIAS "ChannelToolLoader.qml"
)
set_source_files_properties("qml/ChaserStepDelegate.qml"
    PROPERTIES QT_RESOURCE_ALIAS "ChaserStepDelegate.qml"
)
set_source_files_properties("qml/ChaserWidget.qml"
    PROPERTIES QT_RESOURCE_ALIAS "ChaserWidget.qml"
)
set_source_files_properties("qml/ColorTool.qml"
    PROPERTIES QT_RESOURCE_ALIAS "ColorTool.qml"
)
set_source_files_properties("qml/ColorToolBasic.qml"
    PROPERTIES QT_RESOURCE_ALIAS "ColorToolBasic.qml"
)
set_source_files_properties("qml/ColorToolFilters.qml"
    PROPERTIES QT_RESOURCE_ALIAS "ColorToolFilters.qml"
)
set_source_files_properties("qml/ColorToolFull.qml"
    PROPERTIES QT_RESOURCE_ALIAS "ColorToolFull.qml"
)
set_source_files_properties("qml/ColorToolPrimary.qml"
    PROPERTIES QT_RESOURCE_ALIAS "ColorToolPrimary.qml"
)
set_source_files_properties("qml/ContextMenuEntry.qml"
    PROPERTIES QT_RESOURCE_ALIAS "ContextMenuEntry.qml"
)
set_source_files_properties("qml/CustomCheckBox.qml"
    PROPERTIES QT_RESOURCE_ALIAS "CustomCheckBox.qml"
)
set_source_files_properties("qml/CustomComboBox.qml"
    PROPERTIES QT_RESOURCE_ALIAS "CustomComboBox.qml"
)
set_source_files_properties("qml/CustomDoubleSpinBox.qml"
    PROPERTIES QT_RESOURCE_ALIAS "CustomDoubleSpinBox.qml"
)
set_source_files_properties("qml/CustomScrollBar.qml"
    PROPERTIES QT_RESOURCE_ALIAS "CustomScrollBar.qml"
)
set_source_files_properties("qml/CustomSlider.qml"
    PROPERTIES QT_RESOURCE_ALIAS "CustomSlider.qml"
)
set_source_files_properties("qml/CustomSpinBox.qml"
    PROPERTIES QT_RESOURCE_ALIAS "CustomSpinBox.qml"
)
set_source_files_properties("qml/CustomTextEdit.qml"
    PROPERTIES QT_RESOURCE_ALIAS "CustomTextEdit.qml"
)
set_source_files_properties("qml/CustomTextInput.qml"
    PROPERTIES QT_RESOURCE_ALIAS "CustomTextInput.qml"
)
set_source_files_properties("qml/DMXAddressTool.qml"
    PROPERTIES QT_RESOURCE_ALIAS "DMXAddressTool.qml"
)
set_source_files_properties("qml/DMXAddressWidget.qml"
    PROPERTIES QT_RESOURCE_ALIAS "DMXAddressWidget.qml"
)
set_source_files_properties("qml/DMXPercentageButton.qml"
    PROPERTIES QT_RESOURCE_ALIAS "DMXPercentageButton.qml"
)
set_source_files_properties("qml/DayTimeTool.qml"
    PROPERTIES QT_RESOURCE_ALIAS "DayTimeTool.qml"
)
set_source_files_properties("qml/ExternalControlDelegate.qml"
    PROPERTIES QT_RESOURCE_ALIAS "ExternalControlDelegate.qml"
)
set_source_files_properties("qml/ExternalControls.qml"
    PROPERTIES QT_RESOURCE_ALIAS "ExternalControls.qml"
)
set_source_files_properties("qml/FixtureConsole.qml"
    PROPERTIES QT_RESOURCE_ALIAS "FixtureConsole.qml"
)
set_source_files_properties("qml/FixtureDelegate.qml"
    PROPERTIES QT_RESOURCE_ALIAS "FixtureDelegate.qml"
)
set_source_files_properties("qml/FontAwesomeVariables.qml"
    PROPERTIES QT_RESOURCE_ALIAS "FontAwesomeVariables.qml"
)
set_source_files_properties("qml/FunctionDelegate.qml"
    PROPERTIES QT_RESOURCE_ALIAS "FunctionDelegate.qml"
)
set_source_files_properties("qml/GenericButton.qml"
    PROPERTIES QT_RESOURCE_ALIAS "GenericButton.qml"
)
set_source_files_properties("qml/GenericMultiDragItem.qml"
    PROPERTIES QT_RESOURCE_ALIAS "GenericMultiDragItem.qml"
)
set_source_files_properties("qml/IconButton.qml"
    PROPERTIES QT_RESOURCE_ALIAS "IconButton.qml"
)
set_source_files_properties("qml/IconPopupButton.qml"
    PROPERTIES QT_RESOURCE_ALIAS "IconPopupButton.qml"
)
set_source_files_properties("qml/IconTextEntry.qml"
    PROPERTIES QT_RESOURCE_ALIAS "IconTextEntry.qml"
)
set_source_files_properties("qml/InputChannelDelegate.qml"
    PROPERTIES QT_RESOURCE_ALIAS "InputChannelDelegate.qml"
)
set_source_files_properties("qml/KeyPad.qml"
    PROPERTIES QT_RESOURCE_ALIAS "KeyPad.qml"
)
set_source_files_properties("qml/KeyboardSequenceDelegate.qml"
    PROPERTIES QT_RESOURCE_ALIAS "KeyboardSequenceDelegate.qml"
)
set_source_files_properties("qml/MainView.qml"
    PROPERTIES QT_RESOURCE_ALIAS "MainView.qml"
)
set_source_files_properties("qml/MenuBarEntry.qml"
    PROPERTIES QT_RESOURCE_ALIAS "MenuBarEntry.qml"
)
set_source_files_properties("qml/MultiColorBox.qml"
    PROPERTIES QT_RESOURCE_ALIAS "MultiColorBox.qml"
)
set_source_files_properties("qml/PaletteFanningBox.qml"
    PROPERTIES QT_RESOURCE_ALIAS "PaletteFanningBox.qml"
)
set_source_files_properties("qml/QLCPlusFader.qml"
    PROPERTIES QT_RESOURCE_ALIAS "QLCPlusFader.qml"
)
set_source_files_properties("qml/QLCPlusKnob.qml"
    PROPERTIES QT_RESOURCE_ALIAS "QLCPlusKnob.qml"
)
set_source_files_properties("qml/RobotoText.qml"
    PROPERTIES QT_RESOURCE_ALIAS "RobotoText.qml"
)
set_source_files_properties("qml/SectionBox.qml"
    PROPERTIES QT_RESOURCE_ALIAS "SectionBox.qml"
)
set_source_files_properties("qml/SidePanel.qml"
    PROPERTIES QT_RESOURCE_ALIAS "SidePanel.qml"
)
set_source_files_properties("qml/SimpleDesk.qml"
    PROPERTIES QT_RESOURCE_ALIAS "SimpleDesk.qml"
)
set_source_files_properties("qml/SingleAxisTool.qml"
    PROPERTIES QT_RESOURCE_ALIAS "SingleAxisTool.qml"
)
set_source_files_properties("qml/TimeEditTool.qml"
    PROPERTIES QT_RESOURCE_ALIAS "TimeEditTool.qml"
)
set_source_files_properties("qml/TreeNodeDelegate.qml"
    PROPERTIES QT_RESOURCE_ALIAS "TreeNodeDelegate.qml"
)
set_source_files_properties("qml/UISettings.qml"
    PROPERTIES QT_RESOURCE_ALIAS "UISettings.qml"
)
set_source_files_properties("qml/UsageList.qml"
    PROPERTIES QT_RESOURCE_ALIAS "UsageList.qml"
)
set_source_files_properties("qml/WidgetDelegate.qml"
    PROPERTIES QT_RESOURCE_ALIAS "WidgetDelegate.qml"
)
set_source_files_properties("qml/WindowLoader.qml"
    PROPERTIES QT_RESOURCE_ALIAS "WindowLoader.qml"
)
set_source_files_properties("qml/ZoomItem.qml"
    PROPERTIES QT_RESOURCE_ALIAS "ZoomItem.qml"
)
set_source_files_properties("qml/fixtureeditor/ChannelEditor.qml"
    PROPERTIES QT_RESOURCE_ALIAS "ChannelEditor.qml"
)
set_source_files_properties("qml/fixtureeditor/EditorView.qml"
    PROPERTIES QT_RESOURCE_ALIAS "EditorView.qml"
)
set_source_files_properties("qml/fixtureeditor/FixtureEditor.qml"
    PROPERTIES QT_RESOURCE_ALIAS "FixtureEditor.qml"
)
set_source_files_properties("qml/fixtureeditor/ModeEditor.qml"
    PROPERTIES QT_RESOURCE_ALIAS "ModeEditor.qml"
)
set_source_files_properties("qml/fixtureeditor/PhysicalProperties.qml"
    PROPERTIES QT_RESOURCE_ALIAS "PhysicalProperties.qml"
)
set_source_files_properties("qml/fixturesfunctions/2DView.qml"
    PROPERTIES QT_RESOURCE_ALIAS "2DView.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/3DView.qml"
    PROPERTIES QT_RESOURCE_ALIAS "3DView.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/3DViewUnsupported.qml"
    PROPERTIES QT_RESOURCE_ALIAS "3DViewUnsupported.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/BlitEffect.qml"
    PROPERTIES QT_RESOURCE_ALIAS "BlitEffect.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/BlitFilter.qml"
    PROPERTIES QT_RESOURCE_ALIAS "BlitFilter.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/DeferredRenderer.qml"
    PROPERTIES QT_RESOURCE_ALIAS "DeferredRenderer.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/DepthTarget.qml"
    PROPERTIES QT_RESOURCE_ALIAS "DepthTarget.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/DirectionalLightFilter.qml"
    PROPERTIES QT_RESOURCE_ALIAS "DirectionalLightFilter.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/DownsampleEffect.qml"
    PROPERTIES QT_RESOURCE_ALIAS "DownsampleEffect.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/DownsampleFilter.qml"
    PROPERTIES QT_RESOURCE_ALIAS "DownsampleFilter.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/FXAAEffect.qml"
    PROPERTIES QT_RESOURCE_ALIAS "FXAAEffect.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/FXAAFilter.qml"
    PROPERTIES QT_RESOURCE_ALIAS "FXAAFilter.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/FillGBufferFilter.qml"
    PROPERTIES QT_RESOURCE_ALIAS "FillGBufferFilter.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/Fixture3DItem.qml"
    PROPERTIES QT_RESOURCE_ALIAS "Fixture3DItem.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/FrameTarget.qml"
    PROPERTIES QT_RESOURCE_ALIAS "FrameTarget.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/GBuffer.qml"
    PROPERTIES QT_RESOURCE_ALIAS "GBuffer.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/GammaCorrectEffect.qml"
    PROPERTIES QT_RESOURCE_ALIAS "GammaCorrectEffect.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/GammaCorrectFilter.qml"
    PROPERTIES QT_RESOURCE_ALIAS "GammaCorrectFilter.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/Generic3DItem.qml"
    PROPERTIES QT_RESOURCE_ALIAS "Generic3DItem.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/GenericScreenQuadEntity.qml"
    PROPERTIES QT_RESOURCE_ALIAS "GenericScreenQuadEntity.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/GeometryPassEffect.qml"
    PROPERTIES QT_RESOURCE_ALIAS "GeometryPassEffect.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/GrabBrightEffect.qml"
    PROPERTIES QT_RESOURCE_ALIAS "GrabBrightEffect.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/GrabBrightFilter.qml"
    PROPERTIES QT_RESOURCE_ALIAS "GrabBrightFilter.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/LightEntity.qml"
    PROPERTIES QT_RESOURCE_ALIAS "LightEntity.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/LightPassEffect.qml"
    PROPERTIES QT_RESOURCE_ALIAS "LightPassEffect.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/MultiBeams3DItem.qml"
    PROPERTIES QT_RESOURCE_ALIAS "MultiBeams3DItem.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/OutputFrontDepthEffect.qml"
    PROPERTIES QT_RESOURCE_ALIAS "OutputFrontDepthEffect.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/OutputFrontDepthFilter.qml"
    PROPERTIES QT_RESOURCE_ALIAS "OutputFrontDepthFilter.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/PixelBar3DItem.qml"
    PROPERTIES QT_RESOURCE_ALIAS "PixelBar3DItem.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/RenderSelectionBoxesFilter.qml"
    PROPERTIES QT_RESOURCE_ALIAS "RenderSelectionBoxesFilter.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/RenderShadowMapFilter.qml"
    PROPERTIES QT_RESOURCE_ALIAS "RenderShadowMapFilter.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/SceneEntity.qml"
    PROPERTIES QT_RESOURCE_ALIAS "SceneEntity.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/ScreenQuadEntity.qml"
    PROPERTIES QT_RESOURCE_ALIAS "ScreenQuadEntity.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/ScreenQuadGammaCorrectEntity.qml"
    PROPERTIES QT_RESOURCE_ALIAS "ScreenQuadGammaCorrectEntity.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/SelectionEntity.qml"
    PROPERTIES QT_RESOURCE_ALIAS "SelectionEntity.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/SelectionGeometry.qml"
    PROPERTIES QT_RESOURCE_ALIAS "SelectionGeometry.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/SettingsView3D.qml"
    PROPERTIES QT_RESOURCE_ALIAS "SettingsView3D.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/SpotlightConeEntity.qml"
    PROPERTIES QT_RESOURCE_ALIAS "SpotlightConeEntity.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/SpotlightScatteringEffect.qml"
    PROPERTIES QT_RESOURCE_ALIAS "SpotlightScatteringEffect.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/SpotlightScatteringFilter.qml"
    PROPERTIES QT_RESOURCE_ALIAS "SpotlightScatteringFilter.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/SpotlightShadingEffect.qml"
    PROPERTIES QT_RESOURCE_ALIAS "SpotlightShadingEffect.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/SpotlightShadingFilter.qml"
    PROPERTIES QT_RESOURCE_ALIAS "SpotlightShadingFilter.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/StageBox.qml"
    PROPERTIES QT_RESOURCE_ALIAS "StageBox.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/StageRock.qml"
    PROPERTIES QT_RESOURCE_ALIAS "StageRock.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/StageSimple.qml"
    PROPERTIES QT_RESOURCE_ALIAS "StageSimple.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/StageTheatre.qml"
    PROPERTIES QT_RESOURCE_ALIAS "StageTheatre.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/UpsampleEffect.qml"
    PROPERTIES QT_RESOURCE_ALIAS "UpsampleEffect.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/UpsampleFilter.qml"
    PROPERTIES QT_RESOURCE_ALIAS "UpsampleFilter.qml"
)
set_source_files_properties("qml/fixturesfunctions/3DView/shaders/blit.frag"
    PROPERTIES QT_RESOURCE_ALIAS "blit.frag"
)
set_source_files_properties("qml/fixturesfunctions/3DView/shaders/directional.frag"
    PROPERTIES QT_RESOURCE_ALIAS "directional.frag"
)
set_source_files_properties("qml/fixturesfunctions/3DView/shaders/downsample.frag"
    PROPERTIES QT_RESOURCE_ALIAS "downsample.frag"
)
set_source_files_properties("qml/fixturesfunctions/3DView/shaders/fullscreen.vert"
    PROPERTIES QT_RESOURCE_ALIAS "fullscreen.vert"
)
set_source_files_properties("qml/fixturesfunctions/3DView/shaders/fxaa.frag"
    PROPERTIES QT_RESOURCE_ALIAS "fxaa.frag"
)
set_source_files_properties("qml/fixturesfunctions/3DView/shaders/gamma_correct.frag"
    PROPERTIES QT_RESOURCE_ALIAS "gamma_correct.frag"
)
set_source_files_properties("qml/fixturesfunctions/3DView/shaders/geo.frag"
    PROPERTIES QT_RESOURCE_ALIAS "geo.frag"
)
set_source_files_properties("qml/fixturesfunctions/3DView/shaders/geo.vert"
    PROPERTIES QT_RESOURCE_ALIAS "geo.vert"
)
set_source_files_properties("qml/fixturesfunctions/3DView/shaders/grab_bright.frag"
    PROPERTIES QT_RESOURCE_ALIAS "grab_bright.frag"
)
set_source_files_properties("qml/fixturesfunctions/3DView/shaders/output_depth.frag"
    PROPERTIES QT_RESOURCE_ALIAS "output_depth.frag"
)
set_source_files_properties("qml/fixturesfunctions/3DView/shaders/output_depth.vert"
    PROPERTIES QT_RESOURCE_ALIAS "output_depth.vert"
)
set_source_files_properties("qml/fixturesfunctions/3DView/shaders/output_front_depth.frag"
    PROPERTIES QT_RESOURCE_ALIAS "output_front_depth.frag"
)
set_source_files_properties("qml/fixturesfunctions/3DView/shaders/spotlight.vert"
    PROPERTIES QT_RESOURCE_ALIAS "spotlight.vert"
)
set_source_files_properties("qml/fixturesfunctions/3DView/shaders/spotlight_scattering.frag"
    PROPERTIES QT_RESOURCE_ALIAS "spotlight_scattering.frag"
)
set_source_files_properties("qml/fixturesfunctions/3DView/shaders/spotlight_shading.frag"
    PROPERTIES QT_RESOURCE_ALIAS "spotlight_shading.frag"
)
set_source_files_properties("qml/fixturesfunctions/3DView/shaders/upsample.frag"
    PROPERTIES QT_RESOURCE_ALIAS "upsample.frag"
)
set_source_files_properties("qml/fixturesfunctions/AddFunctionMenu.qml"
    PROPERTIES QT_RESOURCE_ALIAS "AddFunctionMenu.qml"
)
set_source_files_properties("qml/fixturesfunctions/AudioEditor.qml"
    PROPERTIES QT_RESOURCE_ALIAS "AudioEditor.qml"
)
set_source_files_properties("qml/fixturesfunctions/BeamTool.qml"
    PROPERTIES QT_RESOURCE_ALIAS "BeamTool.qml"
)
set_source_files_properties("qml/fixturesfunctions/BottomPanel.qml"
    PROPERTIES QT_RESOURCE_ALIAS "BottomPanel.qml"
)
set_source_files_properties("qml/fixturesfunctions/ChaserEditor.qml"
    PROPERTIES QT_RESOURCE_ALIAS "ChaserEditor.qml"
)
set_source_files_properties("qml/fixturesfunctions/CollectionEditor.qml"
    PROPERTIES QT_RESOURCE_ALIAS "CollectionEditor.qml"
)
set_source_files_properties("qml/fixturesfunctions/CollectionFunctionDelegate.qml"
    PROPERTIES QT_RESOURCE_ALIAS "CollectionFunctionDelegate.qml"
)
set_source_files_properties("qml/fixturesfunctions/DMXView.qml"
    PROPERTIES QT_RESOURCE_ALIAS "DMXView.qml"
)
set_source_files_properties("qml/fixturesfunctions/EFXEditor.qml"
    PROPERTIES QT_RESOURCE_ALIAS "EFXEditor.qml"
)
set_source_files_properties("qml/fixturesfunctions/EFXPreview.qml"
    PROPERTIES QT_RESOURCE_ALIAS "EFXPreview.qml"
)
set_source_files_properties("qml/fixturesfunctions/EditorTopBar.qml"
    PROPERTIES QT_RESOURCE_ALIAS "EditorTopBar.qml"
)
set_source_files_properties("qml/fixturesfunctions/Fixture2DItem.qml"
    PROPERTIES QT_RESOURCE_ALIAS "Fixture2DItem.qml"
)
set_source_files_properties("qml/fixturesfunctions/FixtureBrowser.qml"
    PROPERTIES QT_RESOURCE_ALIAS "FixtureBrowser.qml"
)
set_source_files_properties("qml/fixturesfunctions/FixtureBrowserDelegate.qml"
    PROPERTIES QT_RESOURCE_ALIAS "FixtureBrowserDelegate.qml"
)
set_source_files_properties("qml/fixturesfunctions/FixtureChannelDelegate.qml"
    PROPERTIES QT_RESOURCE_ALIAS "FixtureChannelDelegate.qml"
)
set_source_files_properties("qml/fixturesfunctions/FixtureDMXItem.qml"
    PROPERTIES QT_RESOURCE_ALIAS "FixtureDMXItem.qml"
)
set_source_files_properties("qml/fixturesfunctions/FixtureDragItem.qml"
    PROPERTIES QT_RESOURCE_ALIAS "FixtureDragItem.qml"
)
set_source_files_properties("qml/fixturesfunctions/FixtureGroupEditor.qml"
    PROPERTIES QT_RESOURCE_ALIAS "FixtureGroupEditor.qml"
)
set_source_files_properties("qml/fixturesfunctions/FixtureGroupManager.qml"
    PROPERTIES QT_RESOURCE_ALIAS "FixtureGroupManager.qml"
)
set_source_files_properties("qml/fixturesfunctions/FixtureHeadDelegate.qml"
    PROPERTIES QT_RESOURCE_ALIAS "FixtureHeadDelegate.qml"
)
set_source_files_properties("qml/fixturesfunctions/FixtureNodeDelegate.qml"
    PROPERTIES QT_RESOURCE_ALIAS "FixtureNodeDelegate.qml"
)
set_source_files_properties("qml/fixturesfunctions/FixtureProperties.qml"
    PROPERTIES QT_RESOURCE_ALIAS "FixtureProperties.qml"
)
set_source_files_properties("qml/fixturesfunctions/FixtureSummary.qml"
    PROPERTIES QT_RESOURCE_ALIAS "FixtureSummary.qml"
)
set_source_files_properties("qml/fixturesfunctions/FixturesAndFunctions.qml"
    PROPERTIES QT_RESOURCE_ALIAS "FixturesAndFunctions.qml"
)
set_source_files_properties("qml/fixturesfunctions/FunctionManager.qml"
    PROPERTIES QT_RESOURCE_ALIAS "FunctionManager.qml"
)
set_source_files_properties("qml/fixturesfunctions/GridEditor.qml"
    PROPERTIES QT_RESOURCE_ALIAS "GridEditor.qml"
)
set_source_files_properties("qml/fixturesfunctions/IntensityTool.qml"
    PROPERTIES QT_RESOURCE_ALIAS "IntensityTool.qml"
)
set_source_files_properties("qml/fixturesfunctions/LeftPanel.qml"
    PROPERTIES QT_RESOURCE_ALIAS "LeftPanel.qml"
)
set_source_files_properties("qml/fixturesfunctions/PaletteManager.qml"
    PROPERTIES QT_RESOURCE_ALIAS "PaletteManager.qml"
)
set_source_files_properties("qml/fixturesfunctions/PositionTool.qml"
    PROPERTIES QT_RESOURCE_ALIAS "PositionTool.qml"
)
set_source_files_properties("qml/fixturesfunctions/PresetCapabilityItem.qml"
    PROPERTIES QT_RESOURCE_ALIAS "PresetCapabilityItem.qml"
)
set_source_files_properties("qml/fixturesfunctions/PresetsTool.qml"
    PROPERTIES QT_RESOURCE_ALIAS "PresetsTool.qml"
)
set_source_files_properties("qml/fixturesfunctions/RGBMatrixEditor.qml"
    PROPERTIES QT_RESOURCE_ALIAS "RGBMatrixEditor.qml"
)
set_source_files_properties("qml/fixturesfunctions/RGBMatrixPreview.qml"
    PROPERTIES QT_RESOURCE_ALIAS "RGBMatrixPreview.qml"
)
set_source_files_properties("qml/fixturesfunctions/RGBPanelProperties.qml"
    PROPERTIES QT_RESOURCE_ALIAS "RGBPanelProperties.qml"
)
set_source_files_properties("qml/fixturesfunctions/RightPanel.qml"
    PROPERTIES QT_RESOURCE_ALIAS "RightPanel.qml"
)
set_source_files_properties("qml/fixturesfunctions/SceneEditor.qml"
    PROPERTIES QT_RESOURCE_ALIAS "SceneEditor.qml"
)
set_source_files_properties("qml/fixturesfunctions/SceneFixtureConsole.qml"
    PROPERTIES QT_RESOURCE_ALIAS "SceneFixtureConsole.qml"
)
set_source_files_properties("qml/fixturesfunctions/ScriptEditor.qml"
    PROPERTIES QT_RESOURCE_ALIAS "ScriptEditor.qml"
)
set_source_files_properties("qml/fixturesfunctions/SequenceEditor.qml"
    PROPERTIES QT_RESOURCE_ALIAS "SequenceEditor.qml"
)
set_source_files_properties("qml/fixturesfunctions/SettingsView2D.qml"
    PROPERTIES QT_RESOURCE_ALIAS "SettingsView2D.qml"
)
set_source_files_properties("qml/fixturesfunctions/SettingsViewDMX.qml"
    PROPERTIES QT_RESOURCE_ALIAS "SettingsViewDMX.qml"
)
set_source_files_properties("qml/fixturesfunctions/ShutterAnimator.qml"
    PROPERTIES QT_RESOURCE_ALIAS "ShutterAnimator.qml"
)
set_source_files_properties("qml/fixturesfunctions/UniverseGridView.qml"
    PROPERTIES QT_RESOURCE_ALIAS "UniverseGridView.qml"
)
set_source_files_properties("qml/fixturesfunctions/UniverseSummary.qml"
    PROPERTIES QT_RESOURCE_ALIAS "UniverseSummary.qml"
)
set_source_files_properties("qml/fixturesfunctions/VideoContext.qml"
    PROPERTIES QT_RESOURCE_ALIAS "VideoContext.qml"
)
set_source_files_properties("qml/fixturesfunctions/VideoEditor.qml"
    PROPERTIES QT_RESOURCE_ALIAS "VideoEditor.qml"
)
set_source_files_properties("qml/inputoutput/AudioCardsList.qml"
    PROPERTIES QT_RESOURCE_ALIAS "AudioCardsList.qml"
)
set_source_files_properties("qml/inputoutput/AudioDeviceItem.qml"
    PROPERTIES QT_RESOURCE_ALIAS "AudioDeviceItem.qml"
)
set_source_files_properties("qml/inputoutput/AudioIOItem.qml"
    PROPERTIES QT_RESOURCE_ALIAS "AudioIOItem.qml"
)
set_source_files_properties("qml/inputoutput/IOLeftPanel.qml"
    PROPERTIES QT_RESOURCE_ALIAS "IOLeftPanel.qml"
)
set_source_files_properties("qml/inputoutput/IORightPanel.qml"
    PROPERTIES QT_RESOURCE_ALIAS "IORightPanel.qml"
)
set_source_files_properties("qml/inputoutput/InputOutputManager.qml"
    PROPERTIES QT_RESOURCE_ALIAS "InputOutputManager.qml"
)
set_source_files_properties("qml/inputoutput/InputPatchItem.qml"
    PROPERTIES QT_RESOURCE_ALIAS "InputPatchItem.qml"
)
set_source_files_properties("qml/inputoutput/OutputPatchItem.qml"
    PROPERTIES QT_RESOURCE_ALIAS "OutputPatchItem.qml"
)
set_source_files_properties("qml/inputoutput/PatchWireBox.qml"
    PROPERTIES QT_RESOURCE_ALIAS "PatchWireBox.qml"
)
set_source_files_properties("qml/inputoutput/PluginDragItem.qml"
    PROPERTIES QT_RESOURCE_ALIAS "PluginDragItem.qml"
)
set_source_files_properties("qml/inputoutput/PluginsList.qml"
    PROPERTIES QT_RESOURCE_ALIAS "PluginsList.qml"
)
set_source_files_properties("qml/inputoutput/ProfilesList.qml"
    PROPERTIES QT_RESOURCE_ALIAS "ProfilesList.qml"
)
set_source_files_properties("qml/inputoutput/UniverseIOItem.qml"
    PROPERTIES QT_RESOURCE_ALIAS "UniverseIOItem.qml"
)
set_source_files_properties("qml/popup/CustomPopupDialog.qml"
    PROPERTIES QT_RESOURCE_ALIAS "CustomPopupDialog.qml"
)
set_source_files_properties("qml/popup/PopupAbout.qml"
    PROPERTIES QT_RESOURCE_ALIAS "PopupAbout.qml"
)
set_source_files_properties("qml/popup/PopupChannelWizard.qml"
    PROPERTIES QT_RESOURCE_ALIAS "PopupChannelWizard.qml"
)
set_source_files_properties("qml/popup/PopupCreatePalette.qml"
    PROPERTIES QT_RESOURCE_ALIAS "PopupCreatePalette.qml"
)
set_source_files_properties("qml/popup/PopupDMXDump.qml"
    PROPERTIES QT_RESOURCE_ALIAS "PopupDMXDump.qml"
)
set_source_files_properties("qml/popup/PopupDisclaimer.qml"
    PROPERTIES QT_RESOURCE_ALIAS "PopupDisclaimer.qml"
)
set_source_files_properties("qml/popup/PopupImportProject.qml"
    PROPERTIES QT_RESOURCE_ALIAS "PopupImportProject.qml"
)
set_source_files_properties("qml/popup/PopupManualInputSource.qml"
    PROPERTIES QT_RESOURCE_ALIAS "PopupManualInputSource.qml"
)
set_source_files_properties("qml/popup/PopupMonitor.qml"
    PROPERTIES QT_RESOURCE_ALIAS "PopupMonitor.qml"
)
set_source_files_properties("qml/popup/PopupNetworkClient.qml"
    PROPERTIES QT_RESOURCE_ALIAS "PopupNetworkClient.qml"
)
set_source_files_properties("qml/popup/PopupNetworkConnect.qml"
    PROPERTIES QT_RESOURCE_ALIAS "PopupNetworkConnect.qml"
)
set_source_files_properties("qml/popup/PopupNetworkServer.qml"
    PROPERTIES QT_RESOURCE_ALIAS "PopupNetworkServer.qml"
)
set_source_files_properties("qml/popup/PopupPINRequest.qml"
    PROPERTIES QT_RESOURCE_ALIAS "PopupPINRequest.qml"
)
set_source_files_properties("qml/popup/PopupPINSetup.qml"
    PROPERTIES QT_RESOURCE_ALIAS "PopupPINSetup.qml"
)
set_source_files_properties("qml/popup/PopupRenameItems.qml"
    PROPERTIES QT_RESOURCE_ALIAS "PopupRenameItems.qml"
)
set_source_files_properties("qml/qmldir"
    PROPERTIES QT_RESOURCE_ALIAS "qmldir"
)
set_source_files_properties("qml/showmanager/HeaderAndCursor.qml"
    PROPERTIES QT_RESOURCE_ALIAS "HeaderAndCursor.qml"
)
set_source_files_properties("qml/showmanager/ShowItem.qml"
    PROPERTIES QT_RESOURCE_ALIAS "ShowItem.qml"
)
set_source_files_properties("qml/showmanager/ShowManager.qml"
    PROPERTIES QT_RESOURCE_ALIAS "ShowManager.qml"
)
set_source_files_properties("qml/showmanager/TrackDelegate.qml"
    PROPERTIES QT_RESOURCE_ALIAS "TrackDelegate.qml"
)
set_source_files_properties("qml/virtualconsole/VCButtonItem.qml"
    PROPERTIES QT_RESOURCE_ALIAS "VCButtonItem.qml"
)
set_source_files_properties("qml/virtualconsole/VCButtonProperties.qml"
    PROPERTIES QT_RESOURCE_ALIAS "VCButtonProperties.qml"
)
set_source_files_properties("qml/virtualconsole/VCClockItem.qml"
    PROPERTIES QT_RESOURCE_ALIAS "VCClockItem.qml"
)
set_source_files_properties("qml/virtualconsole/VCClockProperties.qml"
    PROPERTIES QT_RESOURCE_ALIAS "VCClockProperties.qml"
)
set_source_files_properties("qml/virtualconsole/VCCueListItem.qml"
    PROPERTIES QT_RESOURCE_ALIAS "VCCueListItem.qml"
)
set_source_files_properties("qml/virtualconsole/VCCueListProperties.qml"
    PROPERTIES QT_RESOURCE_ALIAS "VCCueListProperties.qml"
)
set_source_files_properties("qml/virtualconsole/VCFrameItem.qml"
    PROPERTIES QT_RESOURCE_ALIAS "VCFrameItem.qml"
)
set_source_files_properties("qml/virtualconsole/VCFrameProperties.qml"
    PROPERTIES QT_RESOURCE_ALIAS "VCFrameProperties.qml"
)
set_source_files_properties("qml/virtualconsole/VCLabelItem.qml"
    PROPERTIES QT_RESOURCE_ALIAS "VCLabelItem.qml"
)
set_source_files_properties("qml/virtualconsole/VCPageArea.qml"
    PROPERTIES QT_RESOURCE_ALIAS "VCPageArea.qml"
)
set_source_files_properties("qml/virtualconsole/VCPageProperties.qml"
    PROPERTIES QT_RESOURCE_ALIAS "VCPageProperties.qml"
)
set_source_files_properties("qml/virtualconsole/VCRightPanel.qml"
    PROPERTIES QT_RESOURCE_ALIAS "VCRightPanel.qml"
)
set_source_files_properties("qml/virtualconsole/VCSliderItem.qml"
    PROPERTIES QT_RESOURCE_ALIAS "VCSliderItem.qml"
)
set_source_files_properties("qml/virtualconsole/VCSliderProperties.qml"
    PROPERTIES QT_RESOURCE_ALIAS "VCSliderProperties.qml"
)
set_source_files_properties("qml/virtualconsole/VCWidgetItem.qml"
    PROPERTIES QT_RESOURCE_ALIAS "VCWidgetItem.qml"
)
set_source_files_properties("qml/virtualconsole/VCWidgetProperties.qml"
    PROPERTIES QT_RESOURCE_ALIAS "VCWidgetProperties.qml"
)
set_source_files_properties("qml/virtualconsole/VirtualConsole.qml"
    PROPERTIES QT_RESOURCE_ALIAS "VirtualConsole.qml"
)
set_source_files_properties("qml/virtualconsole/WidgetDragItem.qml"
    PROPERTIES QT_RESOURCE_ALIAS "WidgetDragItem.qml"
)
set_source_files_properties("qml/virtualconsole/WidgetsList.qml"
    PROPERTIES QT_RESOURCE_ALIAS "WidgetsList.qml"
)

target_include_directories(${module_name} PRIVATE
    ../engine/audio/src
    ../engine/src
    ../plugins/interfaces
    fixtureeditor
    tardis
    virtualconsole
)

target_link_libraries(${module_name} PRIVATE
    Qt${QT_MAJOR_VERSION}::3DCore
    Qt${QT_MAJOR_VERSION}::3DInput
    Qt${QT_MAJOR_VERSION}::3DQuick
    Qt${QT_MAJOR_VERSION}::3DQuickExtras
    Qt${QT_MAJOR_VERSION}::3DRender
    Qt${QT_MAJOR_VERSION}::3DExtras
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Multimedia
    Qt${QT_MAJOR_VERSION}::MultimediaWidgets
    Qt${QT_MAJOR_VERSION}::PrintSupport
    Qt${QT_MAJOR_VERSION}::Qml
    Qt${QT_MAJOR_VERSION}::Quick
    Qt${QT_MAJOR_VERSION}::Svg
    Qt${QT_MAJOR_VERSION}::Widgets
    qlcplusengine
)

# Resources:
set_source_files_properties("../resources/icons/svg/2dview.svg"
    PROPERTIES QT_RESOURCE_ALIAS "2dview.svg"
)
set_source_files_properties("../resources/icons/svg/3dview.svg"
    PROPERTIES QT_RESOURCE_ALIAS "3dview.svg"
)
set_source_files_properties("../resources/icons/svg/add.svg"
    PROPERTIES QT_RESOURCE_ALIAS "add.svg"
)
set_source_files_properties("../resources/icons/svg/algo-flat.svg"
    PROPERTIES QT_RESOURCE_ALIAS "algo-flat.svg"
)
set_source_files_properties("../resources/icons/svg/algo-linear.svg"
    PROPERTIES QT_RESOURCE_ALIAS "algo-linear.svg"
)
set_source_files_properties("../resources/icons/svg/algo-saw.svg"
    PROPERTIES QT_RESOURCE_ALIAS "algo-saw.svg"
)
set_source_files_properties("../resources/icons/svg/algo-sine.svg"
    PROPERTIES QT_RESOURCE_ALIAS "algo-sine.svg"
)
set_source_files_properties("../resources/icons/svg/algo-square.svg"
    PROPERTIES QT_RESOURCE_ALIAS "algo-square.svg"
)
set_source_files_properties("../resources/icons/svg/align-bottom.svg"
    PROPERTIES QT_RESOURCE_ALIAS "align-bottom.svg"
)
set_source_files_properties("../resources/icons/svg/align-left.svg"
    PROPERTIES QT_RESOURCE_ALIAS "align-left.svg"
)
set_source_files_properties("../resources/icons/svg/align-right.svg"
    PROPERTIES QT_RESOURCE_ALIAS "align-right.svg"
)
set_source_files_properties("../resources/icons/svg/align-top.svg"
    PROPERTIES QT_RESOURCE_ALIAS "align-top.svg"
)
set_source_files_properties("../resources/icons/svg/amber.svg"
    PROPERTIES QT_RESOURCE_ALIAS "amber.svg"
)
set_source_files_properties("../resources/icons/svg/animation.svg"
    PROPERTIES QT_RESOURCE_ALIAS "animation.svg"
)
set_source_files_properties("../resources/icons/svg/apply.svg"
    PROPERTIES QT_RESOURCE_ALIAS "apply.svg"
)
set_source_files_properties("../resources/icons/svg/arrow-corner.svg"
    PROPERTIES QT_RESOURCE_ALIAS "arrow-corner.svg"
)
set_source_files_properties("../resources/icons/svg/arrow-down.svg"
    PROPERTIES QT_RESOURCE_ALIAS "arrow-down.svg"
)
set_source_files_properties("../resources/icons/svg/arrow-end.svg"
    PROPERTIES QT_RESOURCE_ALIAS "arrow-end.svg"
)
set_source_files_properties("../resources/icons/svg/arrow-right.svg"
    PROPERTIES QT_RESOURCE_ALIAS "arrow-right.svg"
)
set_source_files_properties("../resources/icons/svg/arrow-up.svg"
    PROPERTIES QT_RESOURCE_ALIAS "arrow-up.svg"
)
set_source_files_properties("../resources/icons/svg/artnetplugin.svg"
    PROPERTIES QT_RESOURCE_ALIAS "artnetplugin.svg"
)
set_source_files_properties("../resources/icons/svg/audio.svg"
    PROPERTIES QT_RESOURCE_ALIAS "audio.svg"
)
set_source_files_properties("../resources/icons/svg/audiocard.svg"
    PROPERTIES QT_RESOURCE_ALIAS "audiocard.svg"
)
set_source_files_properties("../resources/icons/svg/audiotriggers.svg"
    PROPERTIES QT_RESOURCE_ALIAS "audiotriggers.svg"
)
set_source_files_properties("../resources/icons/svg/autostart.svg"
    PROPERTIES QT_RESOURCE_ALIAS "autostart.svg"
)
set_source_files_properties("../resources/icons/svg/back.svg"
    PROPERTIES QT_RESOURCE_ALIAS "back.svg"
)
set_source_files_properties("../resources/icons/svg/background.svg"
    PROPERTIES QT_RESOURCE_ALIAS "background.svg"
)
set_source_files_properties("../resources/icons/svg/beam.svg"
    PROPERTIES QT_RESOURCE_ALIAS "beam.svg"
)
set_source_files_properties("../resources/icons/svg/blackout.svg"
    PROPERTIES QT_RESOURCE_ALIAS "blackout.svg"
)
set_source_files_properties("../resources/icons/svg/blue.svg"
    PROPERTIES QT_RESOURCE_ALIAS "blue.svg"
)
set_source_files_properties("../resources/icons/svg/button.svg"
    PROPERTIES QT_RESOURCE_ALIAS "button.svg"
)
set_source_files_properties("../resources/icons/svg/buttonmatrix.svg"
    PROPERTIES QT_RESOURCE_ALIAS "buttonmatrix.svg"
)
set_source_files_properties("../resources/icons/svg/cancel.svg"
    PROPERTIES QT_RESOURCE_ALIAS "cancel.svg"
)
set_source_files_properties("../resources/icons/svg/chaser.svg"
    PROPERTIES QT_RESOURCE_ALIAS "chaser.svg"
)
set_source_files_properties("../resources/icons/svg/clock.svg"
    PROPERTIES QT_RESOURCE_ALIAS "clock.svg"
)
set_source_files_properties("../resources/icons/svg/collection.svg"
    PROPERTIES QT_RESOURCE_ALIAS "collection.svg"
)
set_source_files_properties("../resources/icons/svg/color.svg"
    PROPERTIES QT_RESOURCE_ALIAS "color.svg"
)
set_source_files_properties("../resources/icons/svg/colorwheel.svg"
    PROPERTIES QT_RESOURCE_ALIAS "colorwheel.svg"
)
set_source_files_properties("../resources/icons/svg/configure.svg"
    PROPERTIES QT_RESOURCE_ALIAS "configure.svg"
)
set_source_files_properties("../resources/icons/svg/cuelist.svg"
    PROPERTIES QT_RESOURCE_ALIAS "cuelist.svg"
)
set_source_files_properties("../resources/icons/svg/cyan.svg"
    PROPERTIES QT_RESOURCE_ALIAS "cyan.svg"
)
set_source_files_properties("../resources/icons/svg/dimmer-back.svg"
    PROPERTIES QT_RESOURCE_ALIAS "dimmer-back.svg"
)
set_source_files_properties("../resources/icons/svg/dimmer-fill.svg"
    PROPERTIES QT_RESOURCE_ALIAS "dimmer-fill.svg"
)
set_source_files_properties("../resources/icons/svg/dimmer.svg"
    PROPERTIES QT_RESOURCE_ALIAS "dimmer.svg"
)
set_source_files_properties("../resources/icons/svg/diptool.svg"
    PROPERTIES QT_RESOURCE_ALIAS "diptool.svg"
)
set_source_files_properties("../resources/icons/svg/distribute-x.svg"
    PROPERTIES QT_RESOURCE_ALIAS "distribute-x.svg"
)
set_source_files_properties("../resources/icons/svg/distribute-y.svg"
    PROPERTIES QT_RESOURCE_ALIAS "distribute-y.svg"
)
set_source_files_properties("../resources/icons/svg/dmxdump.svg"
    PROPERTIES QT_RESOURCE_ALIAS "dmxdump.svg"
)
set_source_files_properties("../resources/icons/svg/dmxusbplugin.svg"
    PROPERTIES QT_RESOURCE_ALIAS "dmxusbplugin.svg"
)
set_source_files_properties("../resources/icons/svg/dmxview.svg"
    PROPERTIES QT_RESOURCE_ALIAS "dmxview.svg"
)
set_source_files_properties("../resources/icons/svg/down.svg"
    PROPERTIES QT_RESOURCE_ALIAS "down.svg"
)
set_source_files_properties("../resources/icons/svg/e131plugin.svg"
    PROPERTIES QT_RESOURCE_ALIAS "e131plugin.svg"
)
set_source_files_properties("../resources/icons/svg/edit-copy.svg"
    PROPERTIES QT_RESOURCE_ALIAS "edit-copy.svg"
)
set_source_files_properties("../resources/icons/svg/edit-cut.svg"
    PROPERTIES QT_RESOURCE_ALIAS "edit-cut.svg"
)
set_source_files_properties("../resources/icons/svg/edit-paste.svg"
    PROPERTIES QT_RESOURCE_ALIAS "edit-paste.svg"
)
set_source_files_properties("../resources/icons/svg/edit.svg"
    PROPERTIES QT_RESOURCE_ALIAS "edit.svg"
)
set_source_files_properties("../resources/icons/svg/editor.svg"
    PROPERTIES QT_RESOURCE_ALIAS "editor.svg"
)
set_source_files_properties("../resources/icons/svg/effect.svg"
    PROPERTIES QT_RESOURCE_ALIAS "effect.svg"
)
set_source_files_properties("../resources/icons/svg/efx.svg"
    PROPERTIES QT_RESOURCE_ALIAS "efx.svg"
)
set_source_files_properties("../resources/icons/svg/fan.svg"
    PROPERTIES QT_RESOURCE_ALIAS "fan.svg"
)
set_source_files_properties("../resources/icons/svg/fanning.svg"
    PROPERTIES QT_RESOURCE_ALIAS "fanning.svg"
)
set_source_files_properties("../resources/icons/svg/filenew.svg"
    PROPERTIES QT_RESOURCE_ALIAS "filenew.svg"
)
set_source_files_properties("../resources/icons/svg/fileopen.svg"
    PROPERTIES QT_RESOURCE_ALIAS "fileopen.svg"
)
set_source_files_properties("../resources/icons/svg/filesave.svg"
    PROPERTIES QT_RESOURCE_ALIAS "filesave.svg"
)
set_source_files_properties("../resources/icons/svg/filesaveas.svg"
    PROPERTIES QT_RESOURCE_ALIAS "filesaveas.svg"
)
set_source_files_properties("../resources/icons/svg/fixture.svg"
    PROPERTIES QT_RESOURCE_ALIAS "fixture.svg"
)
set_source_files_properties("../resources/icons/svg/flag_ca.svg"
    PROPERTIES QT_RESOURCE_ALIAS "flag_ca.svg"
)
set_source_files_properties("../resources/icons/svg/flag_de.svg"
    PROPERTIES QT_RESOURCE_ALIAS "flag_de.svg"
)
set_source_files_properties("../resources/icons/svg/flag_es.svg"
    PROPERTIES QT_RESOURCE_ALIAS "flag_es.svg"
)
set_source_files_properties("../resources/icons/svg/flag_fr.svg"
    PROPERTIES QT_RESOURCE_ALIAS "flag_fr.svg"
)
set_source_files_properties("../resources/icons/svg/flag_it.svg"
    PROPERTIES QT_RESOURCE_ALIAS "flag_it.svg"
)
set_source_files_properties("../resources/icons/svg/flag_jp.svg"
    PROPERTIES QT_RESOURCE_ALIAS "flag_jp.svg"
)
set_source_files_properties("../resources/icons/svg/flag_nl.svg"
    PROPERTIES QT_RESOURCE_ALIAS "flag_nl.svg"
)
set_source_files_properties("../resources/icons/svg/flag_pl.svg"
    PROPERTIES QT_RESOURCE_ALIAS "flag_pl.svg"
)
set_source_files_properties("../resources/icons/svg/flag_ru.svg"
    PROPERTIES QT_RESOURCE_ALIAS "flag_ru.svg"
)
set_source_files_properties("../resources/icons/svg/flag_ua.svg"
    PROPERTIES QT_RESOURCE_ALIAS "flag_ua.svg"
)
set_source_files_properties("../resources/icons/svg/flag_uk_us.svg"
    PROPERTIES QT_RESOURCE_ALIAS "flag_uk_us.svg"
)
set_source_files_properties("../resources/icons/svg/flash.svg"
    PROPERTIES QT_RESOURCE_ALIAS "flash.svg"
)
set_source_files_properties("../resources/icons/svg/flower.svg"
    PROPERTIES QT_RESOURCE_ALIAS "flower.svg"
)
set_source_files_properties("../resources/icons/svg/folder.svg"
    PROPERTIES QT_RESOURCE_ALIAS "folder.svg"
)
set_source_files_properties("../resources/icons/svg/font.svg"
    PROPERTIES QT_RESOURCE_ALIAS "font.svg"
)
set_source_files_properties("../resources/icons/svg/forward.svg"
    PROPERTIES QT_RESOURCE_ALIAS "forward.svg"
)
set_source_files_properties("../resources/icons/svg/frame.svg"
    PROPERTIES QT_RESOURCE_ALIAS "frame.svg"
)
set_source_files_properties("../resources/icons/svg/fullscreen.svg"
    PROPERTIES QT_RESOURCE_ALIAS "fullscreen.svg"
)
set_source_files_properties("../resources/icons/svg/functions.svg"
    PROPERTIES QT_RESOURCE_ALIAS "functions.svg"
)
set_source_files_properties("../resources/icons/svg/global.svg"
    PROPERTIES QT_RESOURCE_ALIAS "global.svg"
)
set_source_files_properties("../resources/icons/svg/gobo.svg"
    PROPERTIES QT_RESOURCE_ALIAS "gobo.svg"
)
set_source_files_properties("../resources/icons/svg/green.svg"
    PROPERTIES QT_RESOURCE_ALIAS "green.svg"
)
set_source_files_properties("../resources/icons/svg/grid.svg"
    PROPERTIES QT_RESOURCE_ALIAS "grid.svg"
)
set_source_files_properties("../resources/icons/svg/group.svg"
    PROPERTIES QT_RESOURCE_ALIAS "group.svg"
)
set_source_files_properties("../resources/icons/svg/hazer.svg"
    PROPERTIES QT_RESOURCE_ALIAS "hazer.svg"
)
set_source_files_properties("../resources/icons/svg/help.svg"
    PROPERTIES QT_RESOURCE_ALIAS "help.svg"
)
set_source_files_properties("../resources/icons/svg/hidplugin.svg"
    PROPERTIES QT_RESOURCE_ALIAS "hidplugin.svg"
)
set_source_files_properties("../resources/icons/svg/import.svg"
    PROPERTIES QT_RESOURCE_ALIAS "import.svg"
)
set_source_files_properties("../resources/icons/svg/indigo.svg"
    PROPERTIES QT_RESOURCE_ALIAS "indigo.svg"
)
set_source_files_properties("../resources/icons/svg/info.svg"
    PROPERTIES QT_RESOURCE_ALIAS "info.svg"
)
set_source_files_properties("../resources/icons/svg/inputoutput.svg"
    PROPERTIES QT_RESOURCE_ALIAS "inputoutput.svg"
)
set_source_files_properties("../resources/icons/svg/intensity.svg"
    PROPERTIES QT_RESOURCE_ALIAS "intensity.svg"
)
set_source_files_properties("../resources/icons/svg/keybinding.svg"
    PROPERTIES QT_RESOURCE_ALIAS "keybinding.svg"
)
set_source_files_properties("../resources/icons/svg/knob.svg"
    PROPERTIES QT_RESOURCE_ALIAS "knob.svg"
)
set_source_files_properties("../resources/icons/svg/label.svg"
    PROPERTIES QT_RESOURCE_ALIAS "label.svg"
)
set_source_files_properties("../resources/icons/svg/laser.svg"
    PROPERTIES QT_RESOURCE_ALIAS "laser.svg"
)
set_source_files_properties("../resources/icons/svg/layout-btf.svg"
    PROPERTIES QT_RESOURCE_ALIAS "layout-btf.svg"
)
set_source_files_properties("../resources/icons/svg/layout-btt.svg"
    PROPERTIES QT_RESOURCE_ALIAS "layout-btt.svg"
)
set_source_files_properties("../resources/icons/svg/layout-center.svg"
    PROPERTIES QT_RESOURCE_ALIAS "layout-center.svg"
)
set_source_files_properties("../resources/icons/svg/layout-ftb.svg"
    PROPERTIES QT_RESOURCE_ALIAS "layout-ftb.svg"
)
set_source_files_properties("../resources/icons/svg/layout-ltr.svg"
    PROPERTIES QT_RESOURCE_ALIAS "layout-ltr.svg"
)
set_source_files_properties("../resources/icons/svg/layout-rtl.svg"
    PROPERTIES QT_RESOURCE_ALIAS "layout-rtl.svg"
)
set_source_files_properties("../resources/icons/svg/layout-ttb.svg"
    PROPERTIES QT_RESOURCE_ALIAS "layout-ttb.svg"
)
set_source_files_properties("../resources/icons/svg/ledbar_beams.svg"
    PROPERTIES QT_RESOURCE_ALIAS "ledbar_beams.svg"
)
set_source_files_properties("../resources/icons/svg/ledbar_pixels.svg"
    PROPERTIES QT_RESOURCE_ALIAS "ledbar_pixels.svg"
)
set_source_files_properties("../resources/icons/svg/lime.svg"
    PROPERTIES QT_RESOURCE_ALIAS "lime.svg"
)
set_source_files_properties("../resources/icons/svg/lock.svg"
    PROPERTIES QT_RESOURCE_ALIAS "lock.svg"
)
set_source_files_properties("../resources/icons/svg/loop.svg"
    PROPERTIES QT_RESOURCE_ALIAS "loop.svg"
)
set_source_files_properties("../resources/icons/svg/magenta.svg"
    PROPERTIES QT_RESOURCE_ALIAS "magenta.svg"
)
set_source_files_properties("../resources/icons/svg/midiplugin.svg"
    PROPERTIES QT_RESOURCE_ALIAS "midiplugin.svg"
)
set_source_files_properties("../resources/icons/svg/movinghead.svg"
    PROPERTIES QT_RESOURCE_ALIAS "movinghead.svg"
)
set_source_files_properties("../resources/icons/svg/multiple.svg"
    PROPERTIES QT_RESOURCE_ALIAS "multiple.svg"
)
set_source_files_properties("../resources/icons/svg/network.svg"
    PROPERTIES QT_RESOURCE_ALIAS "network.svg"
)
set_source_files_properties("../resources/icons/svg/olaplugin.svg"
    PROPERTIES QT_RESOURCE_ALIAS "olaplugin.svg"
)
set_source_files_properties("../resources/icons/svg/oscplugin.svg"
    PROPERTIES QT_RESOURCE_ALIAS "oscplugin.svg"
)
set_source_files_properties("../resources/icons/svg/other.svg"
    PROPERTIES QT_RESOURCE_ALIAS "other.svg"
)
set_source_files_properties("../resources/icons/svg/palette.svg"
    PROPERTIES QT_RESOURCE_ALIAS "palette.svg"
)
set_source_files_properties("../resources/icons/svg/pan.svg"
    PROPERTIES QT_RESOURCE_ALIAS "pan.svg"
)
set_source_files_properties("../resources/icons/svg/pause.svg"
    PROPERTIES QT_RESOURCE_ALIAS "pause.svg"
)
set_source_files_properties("../resources/icons/svg/pingpong.svg"
    PROPERTIES QT_RESOURCE_ALIAS "pingpong.svg"
)
set_source_files_properties("../resources/icons/svg/play.svg"
    PROPERTIES QT_RESOURCE_ALIAS "play.svg"
)
set_source_files_properties("../resources/icons/svg/position.svg"
    PROPERTIES QT_RESOURCE_ALIAS "position.svg"
)
set_source_files_properties("../resources/icons/svg/printer.svg"
    PROPERTIES QT_RESOURCE_ALIAS "printer.svg"
)
set_source_files_properties("../resources/icons/svg/prism.svg"
    PROPERTIES QT_RESOURCE_ALIAS "prism.svg"
)
set_source_files_properties("../resources/icons/svg/qlcplus.svg"
    PROPERTIES QT_RESOURCE_ALIAS "qlcplus.svg"
)
set_source_files_properties("../resources/icons/svg/random.svg"
    PROPERTIES QT_RESOURCE_ALIAS "random.svg"
)
set_source_files_properties("../resources/icons/svg/red.svg"
    PROPERTIES QT_RESOURCE_ALIAS "red.svg"
)
set_source_files_properties("../resources/icons/svg/redo.svg"
    PROPERTIES QT_RESOURCE_ALIAS "redo.svg"
)
set_source_files_properties("../resources/icons/svg/remove.svg"
    PROPERTIES QT_RESOURCE_ALIAS "remove.svg"
)
set_source_files_properties("../resources/icons/svg/rename.svg"
    PROPERTIES QT_RESOURCE_ALIAS "rename.svg"
)
set_source_files_properties("../resources/icons/svg/reset.svg"
    PROPERTIES QT_RESOURCE_ALIAS "reset.svg"
)
set_source_files_properties("../resources/icons/svg/resize.svg"
    PROPERTIES QT_RESOURCE_ALIAS "resize.svg"
)
set_source_files_properties("../resources/icons/svg/rgbmatrix.svg"
    PROPERTIES QT_RESOURCE_ALIAS "rgbmatrix.svg"
)
set_source_files_properties("../resources/icons/svg/rotate-right.svg"
    PROPERTIES QT_RESOURCE_ALIAS "rotate-right.svg"
)
set_source_files_properties("../resources/icons/svg/scanner.svg"
    PROPERTIES QT_RESOURCE_ALIAS "scanner.svg"
)
set_source_files_properties("../resources/icons/svg/scene.svg"
    PROPERTIES QT_RESOURCE_ALIAS "scene.svg"
)
set_source_files_properties("../resources/icons/svg/script.svg"
    PROPERTIES QT_RESOURCE_ALIAS "script.svg"
)
set_source_files_properties("../resources/icons/svg/selectall.svg"
    PROPERTIES QT_RESOURCE_ALIAS "selectall.svg"
)
set_source_files_properties("../resources/icons/svg/sequence.svg"
    PROPERTIES QT_RESOURCE_ALIAS "sequence.svg"
)
set_source_files_properties("../resources/icons/svg/showmanager.svg"
    PROPERTIES QT_RESOURCE_ALIAS "showmanager.svg"
)
set_source_files_properties("../resources/icons/svg/shutter.svg"
    PROPERTIES QT_RESOURCE_ALIAS "shutter.svg"
)
set_source_files_properties("../resources/icons/svg/simpledesk.svg"
    PROPERTIES QT_RESOURCE_ALIAS "simpledesk.svg"
)
set_source_files_properties("../resources/icons/svg/slider.svg"
    PROPERTIES QT_RESOURCE_ALIAS "slider.svg"
)
set_source_files_properties("../resources/icons/svg/sliders.svg"
    PROPERTIES QT_RESOURCE_ALIAS "sliders.svg"
)
set_source_files_properties("../resources/icons/svg/smoke.svg"
    PROPERTIES QT_RESOURCE_ALIAS "smoke.svg"
)
set_source_files_properties("../resources/icons/svg/soloframe.svg"
    PROPERTIES QT_RESOURCE_ALIAS "soloframe.svg"
)
set_source_files_properties("../resources/icons/svg/speed.svg"
    PROPERTIES QT_RESOURCE_ALIAS "speed.svg"
)
set_source_files_properties("../resources/icons/svg/star.svg"
    PROPERTIES QT_RESOURCE_ALIAS "star.svg"
)
set_source_files_properties("../resources/icons/svg/stop.svg"
    PROPERTIES QT_RESOURCE_ALIAS "stop.svg"
)
set_source_files_properties("../resources/icons/svg/stopall.svg"
    PROPERTIES QT_RESOURCE_ALIAS "stopall.svg"
)
set_source_files_properties("../resources/icons/svg/stretch.svg"
    PROPERTIES QT_RESOURCE_ALIAS "stretch.svg"
)
set_source_files_properties("../resources/icons/svg/strobe.svg"
    PROPERTIES QT_RESOURCE_ALIAS "strobe.svg"
)
set_source_files_properties("../resources/icons/svg/tilt.svg"
    PROPERTIES QT_RESOURCE_ALIAS "tilt.svg"
)
set_source_files_properties("../resources/icons/svg/uncheck.svg"
    PROPERTIES QT_RESOURCE_ALIAS "uncheck.svg"
)
set_source_files_properties("../resources/icons/svg/undo.svg"
    PROPERTIES QT_RESOURCE_ALIAS "undo.svg"
)
set_source_files_properties("../resources/icons/svg/uniview.svg"
    PROPERTIES QT_RESOURCE_ALIAS "uniview.svg"
)
set_source_files_properties("../resources/icons/svg/unlock.svg"
    PROPERTIES QT_RESOURCE_ALIAS "unlock.svg"
)
set_source_files_properties("../resources/icons/svg/up.svg"
    PROPERTIES QT_RESOURCE_ALIAS "up.svg"
)
set_source_files_properties("../resources/icons/svg/uv.svg"
    PROPERTIES QT_RESOURCE_ALIAS "uv.svg"
)
set_source_files_properties("../resources/icons/svg/video.svg"
    PROPERTIES QT_RESOURCE_ALIAS "video.svg"
)
set_source_files_properties("../resources/icons/svg/virtualconsole.svg"
    PROPERTIES QT_RESOURCE_ALIAS "virtualconsole.svg"
)
set_source_files_properties("../resources/icons/svg/white.svg"
    PROPERTIES QT_RESOURCE_ALIAS "white.svg"
)
set_source_files_properties("../resources/icons/svg/wizard.svg"
    PROPERTIES QT_RESOURCE_ALIAS "wizard.svg"
)
set_source_files_properties("../resources/icons/svg/xypad.svg"
    PROPERTIES QT_RESOURCE_ALIAS "xypad.svg"
)
set_source_files_properties("../resources/icons/svg/yellow.svg"
    PROPERTIES QT_RESOURCE_ALIAS "yellow.svg"
)
set(svgicons_resource_files
    "../resources/icons/svg/2dview.svg"
    "../resources/icons/svg/3dview.svg"
    "../resources/icons/svg/add.svg"
    "../resources/icons/svg/algo-flat.svg"
    "../resources/icons/svg/algo-linear.svg"
    "../resources/icons/svg/algo-saw.svg"
    "../resources/icons/svg/algo-sine.svg"
    "../resources/icons/svg/algo-square.svg"
    "../resources/icons/svg/align-bottom.svg"
    "../resources/icons/svg/align-left.svg"
    "../resources/icons/svg/align-right.svg"
    "../resources/icons/svg/align-top.svg"
    "../resources/icons/svg/amber.svg"
    "../resources/icons/svg/animation.svg"
    "../resources/icons/svg/apply.svg"
    "../resources/icons/svg/arrow-corner.svg"
    "../resources/icons/svg/arrow-down.svg"
    "../resources/icons/svg/arrow-end.svg"
    "../resources/icons/svg/arrow-right.svg"
    "../resources/icons/svg/arrow-up.svg"
    "../resources/icons/svg/artnetplugin.svg"
    "../resources/icons/svg/audio.svg"
    "../resources/icons/svg/audiocard.svg"
    "../resources/icons/svg/audiotriggers.svg"
    "../resources/icons/svg/autostart.svg"
    "../resources/icons/svg/back.svg"
    "../resources/icons/svg/background.svg"
    "../resources/icons/svg/beam.svg"
    "../resources/icons/svg/blackout.svg"
    "../resources/icons/svg/blue.svg"
    "../resources/icons/svg/button.svg"
    "../resources/icons/svg/buttonmatrix.svg"
    "../resources/icons/svg/cancel.svg"
    "../resources/icons/svg/chaser.svg"
    "../resources/icons/svg/clock.svg"
    "../resources/icons/svg/collection.svg"
    "../resources/icons/svg/color.svg"
    "../resources/icons/svg/colorwheel.svg"
    "../resources/icons/svg/configure.svg"
    "../resources/icons/svg/cuelist.svg"
    "../resources/icons/svg/cyan.svg"
    "../resources/icons/svg/dimmer-back.svg"
    "../resources/icons/svg/dimmer-fill.svg"
    "../resources/icons/svg/dimmer.svg"
    "../resources/icons/svg/diptool.svg"
    "../resources/icons/svg/distribute-x.svg"
    "../resources/icons/svg/distribute-y.svg"
    "../resources/icons/svg/dmxdump.svg"
    "../resources/icons/svg/dmxusbplugin.svg"
    "../resources/icons/svg/dmxview.svg"
    "../resources/icons/svg/down.svg"
    "../resources/icons/svg/e131plugin.svg"
    "../resources/icons/svg/edit-copy.svg"
    "../resources/icons/svg/edit-cut.svg"
    "../resources/icons/svg/edit-paste.svg"
    "../resources/icons/svg/edit.svg"
    "../resources/icons/svg/editor.svg"
    "../resources/icons/svg/effect.svg"
    "../resources/icons/svg/efx.svg"
    "../resources/icons/svg/fan.svg"
    "../resources/icons/svg/fanning.svg"
    "../resources/icons/svg/filenew.svg"
    "../resources/icons/svg/fileopen.svg"
    "../resources/icons/svg/filesave.svg"
    "../resources/icons/svg/filesaveas.svg"
    "../resources/icons/svg/fixture.svg"
    "../resources/icons/svg/flag_ca.svg"
    "../resources/icons/svg/flag_de.svg"
    "../resources/icons/svg/flag_es.svg"
    "../resources/icons/svg/flag_fr.svg"
    "../resources/icons/svg/flag_it.svg"
    "../resources/icons/svg/flag_jp.svg"
    "../resources/icons/svg/flag_nl.svg"
    "../resources/icons/svg/flag_pl.svg"
    "../resources/icons/svg/flag_ru.svg"
    "../resources/icons/svg/flag_ua.svg"
    "../resources/icons/svg/flag_uk_us.svg"
    "../resources/icons/svg/flash.svg"
    "../resources/icons/svg/flower.svg"
    "../resources/icons/svg/folder.svg"
    "../resources/icons/svg/font.svg"
    "../resources/icons/svg/forward.svg"
    "../resources/icons/svg/frame.svg"
    "../resources/icons/svg/fullscreen.svg"
    "../resources/icons/svg/functions.svg"
    "../resources/icons/svg/global.svg"
    "../resources/icons/svg/gobo.svg"
    "../resources/icons/svg/green.svg"
    "../resources/icons/svg/grid.svg"
    "../resources/icons/svg/group.svg"
    "../resources/icons/svg/hazer.svg"
    "../resources/icons/svg/help.svg"
    "../resources/icons/svg/hidplugin.svg"
    "../resources/icons/svg/import.svg"
    "../resources/icons/svg/indigo.svg"
    "../resources/icons/svg/info.svg"
    "../resources/icons/svg/inputoutput.svg"
    "../resources/icons/svg/intensity.svg"
    "../resources/icons/svg/keybinding.svg"
    "../resources/icons/svg/knob.svg"
    "../resources/icons/svg/label.svg"
    "../resources/icons/svg/laser.svg"
    "../resources/icons/svg/layout-btf.svg"
    "../resources/icons/svg/layout-btt.svg"
    "../resources/icons/svg/layout-center.svg"
    "../resources/icons/svg/layout-ftb.svg"
    "../resources/icons/svg/layout-ltr.svg"
    "../resources/icons/svg/layout-rtl.svg"
    "../resources/icons/svg/layout-ttb.svg"
    "../resources/icons/svg/ledbar_beams.svg"
    "../resources/icons/svg/ledbar_pixels.svg"
    "../resources/icons/svg/lime.svg"
    "../resources/icons/svg/lock.svg"
    "../resources/icons/svg/loop.svg"
    "../resources/icons/svg/magenta.svg"
    "../resources/icons/svg/midiplugin.svg"
    "../resources/icons/svg/movinghead.svg"
    "../resources/icons/svg/multiple.svg"
    "../resources/icons/svg/network.svg"
    "../resources/icons/svg/olaplugin.svg"
    "../resources/icons/svg/oscplugin.svg"
    "../resources/icons/svg/other.svg"
    "../resources/icons/svg/palette.svg"
    "../resources/icons/svg/pan.svg"
    "../resources/icons/svg/pause.svg"
    "../resources/icons/svg/pingpong.svg"
    "../resources/icons/svg/play.svg"
    "../resources/icons/svg/position.svg"
    "../resources/icons/svg/printer.svg"
    "../resources/icons/svg/prism.svg"
    "../resources/icons/svg/qlcplus.svg"
    "../resources/icons/svg/random.svg"
    "../resources/icons/svg/red.svg"
    "../resources/icons/svg/redo.svg"
    "../resources/icons/svg/remove.svg"
    "../resources/icons/svg/rename.svg"
    "../resources/icons/svg/reset.svg"
    "../resources/icons/svg/resize.svg"
    "../resources/icons/svg/rgbmatrix.svg"
    "../resources/icons/svg/rotate-right.svg"
    "../resources/icons/svg/scanner.svg"
    "../resources/icons/svg/scene.svg"
    "../resources/icons/svg/script.svg"
    "../resources/icons/svg/selectall.svg"
    "../resources/icons/svg/sequence.svg"
    "../resources/icons/svg/showmanager.svg"
    "../resources/icons/svg/shutter.svg"
    "../resources/icons/svg/simpledesk.svg"
    "../resources/icons/svg/slider.svg"
    "../resources/icons/svg/sliders.svg"
    "../resources/icons/svg/smoke.svg"
    "../resources/icons/svg/soloframe.svg"
    "../resources/icons/svg/speed.svg"
    "../resources/icons/svg/star.svg"
    "../resources/icons/svg/stop.svg"
    "../resources/icons/svg/stopall.svg"
    "../resources/icons/svg/stretch.svg"
    "../resources/icons/svg/strobe.svg"
    "../resources/icons/svg/tilt.svg"
    "../resources/icons/svg/uncheck.svg"
    "../resources/icons/svg/undo.svg"
    "../resources/icons/svg/uniview.svg"
    "../resources/icons/svg/unlock.svg"
    "../resources/icons/svg/up.svg"
    "../resources/icons/svg/uv.svg"
    "../resources/icons/svg/video.svg"
    "../resources/icons/svg/virtualconsole.svg"
    "../resources/icons/svg/white.svg"
    "../resources/icons/svg/wizard.svg"
    "../resources/icons/svg/xypad.svg"
    "../resources/icons/svg/yellow.svg"
)

if(QT_VERSION_MAJOR GREATER 5)
    qt_add_resources(${module_name} "svgicons"
        PREFIX
            "/"
        BASE
            "../resources/icons/svg"
        FILES
            ${svgicons_resource_files}
    )
endif()

set_source_files_properties("../resources/fonts/FontAwesome.otf"
    PROPERTIES QT_RESOURCE_ALIAS "FontAwesome.otf"
)
set_source_files_properties("../resources/fonts/RobotoCondensed-Regular.ttf"
    PROPERTIES QT_RESOURCE_ALIAS "RobotoCondensed-Regular.ttf"
)
set_source_files_properties("../resources/fonts/RobotoMono-Regular.ttf"
    PROPERTIES QT_RESOURCE_ALIAS "RobotoMono-Regular.ttf"
)
set(fonts_resource_files
    "../resources/fonts/FontAwesome.otf"
    "../resources/fonts/RobotoCondensed-Regular.ttf"
    "../resources/fonts/RobotoMono-Regular.ttf"
)

if(QT_VERSION_MAJOR GREATER 5)
    qt_add_resources(${module_name} "fonts"
        PREFIX
            "/"
        BASE
            "../resources/fonts"
        FILES
            ${fonts_resource_files}
    )
endif()

if(lupdate_only)
    target_sources(${module_name} PRIVATE
        qml/*.qml
        qml/fixturesfunctions/*.qml
        qml/inputoutput/*.qml
        qml/popup/*.qml
        qml/showmanager/*.qml
        qml/virtualconsole/*.qml
    )
endif()

if(QT_VERSION_MAJOR EQUAL 5)
    qt5_add_resources(qlcplusqmlui_resource_files qmlui.qrc ../resources/icons/svg/svgicons.qrc ../resources/fonts/fonts.qrc)
    target_sources(${module_name} PUBLIC
        ${qlcplusqmlui_resource_files}
    )
endif()

install(TARGETS ${module_name}
    DESTINATION ${INSTALLROOT}/${BINDIR}
)
