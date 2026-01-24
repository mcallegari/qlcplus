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

set(QML_IMPORT_PATH ".")

if(QT_VERSION_MAJOR GREATER 5)
    qt_add_translation(QM_FILES ${TS_FILES})
else()
    qt5_add_translation(QM_FILES ${TS_FILES})
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
    virtualconsole/vcspeeddial.cpp virtualconsole/vcspeeddial.h
    virtualconsole/vcspeeddialpreset.cpp virtualconsole/vcspeeddialpreset.h
    virtualconsole/virtualconsole.cpp virtualconsole/virtualconsole.h
)

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
target_include_directories(${module_name} PUBLIC ${FFTW3_INCLUDE_DIRS})

target_compile_definitions(${module_name} PRIVATE
    NO_SSL
    USE_WEBSOCKET
)

# Workaround a nasty Debian/Ubuntu Qt3d packaging issue where
# a non existent path would break the cmake initialization 
set(_qt6_3d_modules 3DCore 3DInput 3DQuick 3DQuickExtras 3DRender 3DExtras)
set(_qt6_3d_link "")
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

install(TARGETS ${module_name} DESTINATION ${INSTALLROOT}/${BINDIR})
