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
    inputprofileeditor.cpp inputprofileeditor.h
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
    ../plugins/midi/src/common
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
else()
    qt_add_resources(qlcplusqmlui_resource_files qmlui.qrc ../resources/icons/svg/svgicons.qrc ../resources/fonts/fonts.qrc)
    target_sources(${module_name} PUBLIC
        ${qlcplusqmlui_resource_files}
    )
endif()

install(TARGETS ${module_name}
    DESTINATION ${INSTALLROOT}/${BINDIR}
)
