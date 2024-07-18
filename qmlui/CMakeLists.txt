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

set(SRC_FILES
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
    virtualconsole/vcanimation.cpp virtualconsole/vcanimation.h
    virtualconsole/vcaudiotrigger.cpp virtualconsole/vcaudiotrigger.h
    virtualconsole/vcbutton.cpp virtualconsole/vcbutton.h
    virtualconsole/vcclock.cpp virtualconsole/vcclock.h
    virtualconsole/vccuelist.cpp virtualconsole/vccuelist.h
    virtualconsole/vcframe.cpp virtualconsole/vcframe.h
    virtualconsole/vclabel.cpp virtualconsole/vclabel.h
    virtualconsole/vcpage.cpp virtualconsole/vcpage.h
    virtualconsole/vcslider.cpp virtualconsole/vcslider.h
    virtualconsole/vcsoloframe.cpp virtualconsole/vcsoloframe.h
    virtualconsole/vcwidget.cpp virtualconsole/vcwidget.h
    virtualconsole/vcxypad.cpp virtualconsole/vcxypad.h
    virtualconsole/vcspeeddial.cpp virtualconsole/vcspeeddial.h
    virtualconsole/virtualconsole.cpp virtualconsole/virtualconsole.h
)

if(ANDROID)
    add_library(${module_name} SHARED
        ${SRC_FILES}
        ${QM_FILES}
    )
    set_target_properties(
        ${module_name}
        PROPERTIES LIBRARY_OUTPUT_NAME qlcplus)
else()
    add_executable(${module_name} WIN32
        ${SRC_FILES}
        ${QM_FILES}
    )
endif()

if(WIN32)
    target_sources(${module_name} PRIVATE
        qmlui.rc
    )
endif()

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

if(ANDROID)
    target_link_libraries(${module_name} PRIVATE
        Qt${QT_MAJOR_VERSION}::Concurrent
        Qt${QT_MAJOR_VERSION}::OpenGL
        GLESv2
        log
        z
        c++_shared
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
else()
    qt_add_resources(qlcplusqmlui_resource_files qmlui.qrc ../resources/icons/svg/svgicons.qrc ../resources/fonts/fonts.qrc)
    target_sources(${module_name} PUBLIC
        ${qlcplusqmlui_resource_files}
    )
endif()

install(TARGETS ${module_name}
    DESTINATION ${INSTALLROOT}/${BINDIR}
)
