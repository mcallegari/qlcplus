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
qt_add_translation(QM_FILES ${TS_FILES})

set(QML_IMPORT_PATH ".")

if(RHI3D)
    find_package(Qt${QT_MAJOR_VERSION} REQUIRED COMPONENTS GuiPrivate ShaderTools)

    set(QLCPLUS_MAINVIEW3D_SOURCE mainview3drhi.cpp)
    set(QLCPLUS_MAINVIEW3D_HEADER mainview3drhi.h)
else()
    set(QLCPLUS_MAINVIEW3D_SOURCE mainview3d.cpp)
    set(QLCPLUS_MAINVIEW3D_HEADER mainview3dqt3d.h)
endif()

set(SRC_FILES
    ../plugins/interfaces/utils.h
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
    ${QLCPLUS_MAINVIEW3D_SOURCE} ${QLCPLUS_MAINVIEW3D_HEADER} mainview3d.h
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
    waveformimageprovider.cpp waveformimageprovider.h
    virtualconsole/vcanimation.cpp virtualconsole/vcanimation.h
    virtualconsole/vcaudiotriggers.cpp virtualconsole/vcaudiotriggers.h
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
    virtualconsole/vcxypadpreset.cpp virtualconsole/vcxypadpreset.h
    virtualconsole/vcspeeddial.cpp virtualconsole/vcspeeddial.h
    virtualconsole/vcspeeddialpreset.cpp virtualconsole/vcspeeddialpreset.h
    virtualconsole/virtualconsole.cpp virtualconsole/virtualconsole.h
)

if(RHI3D)
    list(APPEND SRC_FILES
        rhi3d/core/RhiContext.cpp rhi3d/core/RhiContext.h
        rhi3d/core/RenderGraph.cpp rhi3d/core/RenderGraph.h
        rhi3d/core/RenderTargetCache.cpp rhi3d/core/RenderTargetCache.h
        rhi3d/core/ShaderManager.cpp rhi3d/core/ShaderManager.h
        rhi3d/qml/CameraItem.cpp rhi3d/qml/CameraItem.h
        rhi3d/qml/CubeItem.cpp rhi3d/qml/CubeItem.h
        rhi3d/qml/BeamBarItem.cpp rhi3d/qml/BeamBarItem.h
        rhi3d/qml/HazerItem.cpp rhi3d/qml/HazerItem.h
        rhi3d/qml/LightItem.cpp rhi3d/qml/LightItem.h
        rhi3d/qml/MeshItem.cpp rhi3d/qml/MeshItem.h
        rhi3d/qml/MeshUtils.cpp rhi3d/qml/MeshUtils.h
        rhi3d/qml/MovingHeadItem.cpp rhi3d/qml/MovingHeadItem.h
        rhi3d/qml/ModelItem.cpp rhi3d/qml/ModelItem.h
        rhi3d/qml/PixelBarItem.cpp rhi3d/qml/PixelBarItem.h
        rhi3d/qml/PickingUtils.cpp rhi3d/qml/PickingUtils.h
        rhi3d/qml/RhiQmlItem.cpp rhi3d/qml/RhiQmlItem.h
        rhi3d/qml/SphereItem.cpp rhi3d/qml/SphereItem.h
        rhi3d/qml/StaticLightItem.cpp rhi3d/qml/StaticLightItem.h
        rhi3d/qml/VideoItem.cpp rhi3d/qml/VideoItem.h
        rhi3d/renderer/DeferredRenderer.cpp rhi3d/renderer/DeferredRenderer.h
        rhi3d/renderer/PassDepth.cpp rhi3d/renderer/PassDepth.h
        rhi3d/renderer/PassGBuffer.cpp rhi3d/renderer/PassGBuffer.h
        rhi3d/renderer/PassLightCulling.cpp rhi3d/renderer/PassLightCulling.h
        rhi3d/renderer/PassLighting.cpp rhi3d/renderer/PassLighting.h
        rhi3d/renderer/PassShadow.cpp rhi3d/renderer/PassShadow.h
        rhi3d/renderer/PassPost.cpp rhi3d/renderer/PassPost.h
        rhi3d/scene/AssimpLoader.cpp rhi3d/scene/AssimpLoader.h
        rhi3d/scene/Camera.cpp rhi3d/scene/Camera.h
        rhi3d/scene/Material.cpp rhi3d/scene/Material.h
        rhi3d/scene/Mesh.cpp rhi3d/scene/Mesh.h
        rhi3d/scene/Scene.cpp rhi3d/scene/Scene.h
    )
endif()

# This is most of the Android deployment code
if(ANDROID)
    qt_add_executable(${module_name} ${SRC_FILES} ${QM_FILES})
    set_target_properties(${module_name} PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR "${ANDROID_PACKAGE_STAGING_DIR}"
        QT_ANDROID_APP_ICON "@drawable/ic_launcher"
        QT_ANDROID_PACKAGE_NAME "org.qlcplus"
        OUTPUT_NAME "QLC+"
    )

    # Copy overlay resources into build tree
    file(MAKE_DIRECTORY "${ANDROID_PACKAGE_STAGING_DIR}")
    file(COPY "${PROJECT_SOURCE_DIR}/platforms/android/" DESTINATION "${ANDROID_PACKAGE_STAGING_DIR}")

    qt_policy(SET QTP0002 NEW)
    set_property(TARGET ${module_name} APPEND PROPERTY QT_ANDROID_EXTRA_LIBS "$<TARGET_FILE:artnet>")
    set_property(TARGET ${module_name} APPEND PROPERTY QT_ANDROID_EXTRA_LIBS "$<TARGET_FILE:e131>")
    set_property(TARGET ${module_name} APPEND PROPERTY QT_ANDROID_EXTRA_LIBS "$<TARGET_FILE:loopback>")
    set_property(TARGET ${module_name} APPEND PROPERTY QT_ANDROID_EXTRA_LIBS "$<TARGET_FILE:osc>")
    set_property(TARGET ${module_name} APPEND PROPERTY QT_ANDROID_EXTRA_LIBS "$<TARGET_FILE:os2l>")
else()
    add_executable(${module_name} WIN32 ${SRC_FILES} ${QM_FILES})
endif()

if(WIN32)
    target_sources(${module_name} PRIVATE qmlui.rc)
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
if(RHI3D)
    target_include_directories(${module_name} PRIVATE
        rhi3d
    )
endif()
target_include_directories(${module_name} PUBLIC ${FFTW3_INCLUDE_DIRS})

target_compile_definitions(${module_name} PRIVATE
    NO_SSL
    USE_WEBSOCKET
)
if(RHI3D)
    target_compile_definitions(${module_name} PRIVATE RHI3D)
endif()

set(_qt6_3d_link "")
if(NOT RHI3D)
    # Workaround a nasty Debian/Ubuntu Qt3d packaging issue where
    # a non existent path would break the cmake initialization
    set(_qt6_3d_modules 3DCore 3DInput 3DLogic 3DQuick 3DQuickExtras 3DRender 3DExtras)
    foreach(_m IN LISTS _qt6_3d_modules)
        list(APPEND _qt6_3d_link "Qt${QT_MAJOR_VERSION}::${_m}")
    endforeach()
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND QT_MAJOR_VERSION EQUAL 6 AND (NOT DEFINED ENV{QTDIR} OR "$ENV{QTDIR}" STREQUAL ""))
        set(_qt6_base "")
        get_target_property(_qt6_core_incs Qt6::Core INTERFACE_INCLUDE_DIRECTORIES)
        if(_qt6_core_incs AND NOT _qt6_core_incs MATCHES "-NOTFOUND$")
            foreach(inc IN LISTS _qt6_core_incs)
                if(inc MATCHES "/qt6$")
                    set(_qt6_base "${inc}")
                    break()
                elseif(inc MATCHES "/qt6/QtCore$")
                    get_filename_component(_qt6_base "${inc}" DIRECTORY)
                    break()
                endif()
            endforeach()
        endif()
        if(_qt6_base)
            target_include_directories(${module_name} PRIVATE
                ${_qt6_base}
            )
            foreach(_m IN LISTS _qt6_3d_modules)
                target_include_directories(${module_name} PRIVATE
                    ${_qt6_base}/Qt${_m}
                )
            endforeach()
            function(_qt6_get_imported_location tgt outvar)
                set(_loc "")
                if(CMAKE_BUILD_TYPE)
                    string(TOUPPER "${CMAKE_BUILD_TYPE}" _cfg)
                    get_target_property(_loc "${tgt}" "IMPORTED_LOCATION_${_cfg}")
                endif()
                if(NOT _loc OR _loc MATCHES "-NOTFOUND$")
                    get_target_property(_loc "${tgt}" "IMPORTED_LOCATION")
                endif()
                if(NOT _loc OR _loc MATCHES "-NOTFOUND$")
                    foreach(_cfg IN ITEMS RELEASE RELWITHDEBINFO MINSIZEREL DEBUG)
                        get_target_property(_loc "${tgt}" "IMPORTED_LOCATION_${_cfg}")
                        if(_loc AND NOT _loc MATCHES "-NOTFOUND$")
                            break()
                        endif()
                    endforeach()
                endif()
                if(_loc AND NOT _loc MATCHES "-NOTFOUND$")
                    set(${outvar} "${_loc}" PARENT_SCOPE)
                else()
                    set(${outvar} "" PARENT_SCOPE)
                endif()
            endfunction()
            set(_qt6_3d_link "")
            foreach(_m IN LISTS _qt6_3d_modules)
                set(_tgt "Qt6::${_m}")
                if(TARGET "${_tgt}")
                    _qt6_get_imported_location("${_tgt}" _loc)
                    if(_loc)
                        list(APPEND _qt6_3d_link "${_loc}")
                    else()
                        list(APPEND _qt6_3d_link "${_tgt}")
                    endif()
                endif()
            endforeach()
        endif()
    endif()
endif()

target_link_libraries(${module_name} PRIVATE
    ${_qt6_3d_link}
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Multimedia
    Qt${QT_MAJOR_VERSION}::MultimediaWidgets
    Qt${QT_MAJOR_VERSION}::Network
    Qt${QT_MAJOR_VERSION}::PrintSupport
    Qt${QT_MAJOR_VERSION}::Qml
    Qt${QT_MAJOR_VERSION}::Quick
    Qt${QT_MAJOR_VERSION}::Svg
    Qt${QT_MAJOR_VERSION}::Widgets
    Qt${QT_MAJOR_VERSION}::WebSockets
    qlcplusengine
    qlcpluswebaccess
)

if(RHI3D)
    find_package(assimp QUIET)
    target_compile_features(${module_name} PRIVATE cxx_std_17)
    target_link_libraries(${module_name} PRIVATE
        Qt${QT_MAJOR_VERSION}::GuiPrivate
    )
    target_compile_definitions(${module_name} PRIVATE
        GOBO_PATH="${PROJECT_SOURCE_DIR}/resources/gobos"
    )
    if(assimp_FOUND)
        target_link_libraries(${module_name} PRIVATE assimp::assimp)
    else()
        target_compile_definitions(${module_name} PRIVATE RHIPIPELINE_NO_ASSIMP)
    endif()
endif()

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

if(RHI3D)
    qt_add_shaders(${module_name} qlcplus_rhi_shaders
        PREFIX "/shaders"
        BASE "${CMAKE_CURRENT_SOURCE_DIR}/rhi3d/shaders"
        GLSL "430,310es"
        FILES
            rhi3d/shaders/gbuffer.vert
            rhi3d/shaders/gbuffer.frag
            rhi3d/shaders/gizmo.vert
            rhi3d/shaders/gizmo.frag
            rhi3d/shaders/lighting.vert
            rhi3d/shaders/lighting.frag
            rhi3d/shaders/lighting_d3d.frag
            rhi3d/shaders/lighting_metal.frag
            rhi3d/shaders/lighting_cull.frag
            rhi3d/shaders/lighting_cull_d3d.frag
            rhi3d/shaders/lighting_cull_metal.frag
            rhi3d/shaders/post_bloom_downsample.frag
            rhi3d/shaders/post_bloom_upsample.frag
            rhi3d/shaders/post_combine.frag
            rhi3d/shaders/selection_box.vert
            rhi3d/shaders/selection_box.frag
            rhi3d/shaders/shadow.vert
            rhi3d/shaders/shadow.frag
            rhi3d/shaders/shadow_spot.frag
            rhi3d/shaders/tonemap.frag
    )
    qt_add_shaders(${module_name} qlcplus_rhi_compute_shaders
        PREFIX "/shaders"
        BASE "${CMAKE_CURRENT_SOURCE_DIR}/rhi3d/shaders"
        GLSL "310es,430"
        FILES
            rhi3d/shaders/light_cull.comp
    )
endif()

qt_add_resources(qlcplusqmlui_resource_files qmlui.qrc ../resources/icons/svg/svgicons.qrc ../resources/fonts/fonts.qrc)
target_sources(${module_name} PUBLIC
    ${qlcplusqmlui_resource_files}
)

install(TARGETS ${module_name} DESTINATION ${INSTALLROOT}/${BINDIR})
