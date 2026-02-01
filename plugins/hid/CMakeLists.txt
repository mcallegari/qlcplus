set(module_name "hidplugin")

set(TS_FILES
    HID_fi_FI.ts
    HID_de_DE.ts
    HID_es_ES.ts
    HID_fr_FR.ts
    HID_it_IT.ts
    HID_nl_NL.ts
    HID_cz_CZ.ts
    HID_pt_BR.ts
    HID_ca_ES.ts
    HID_ja_JP.ts
)

if(QT_VERSION_MAJOR GREATER 5)
    qt_add_translation(QM_FILES ${TS_FILES})
else()
    qt5_add_translation(QM_FILES ${TS_FILES})
endif()

add_library(${module_name} SHARED
    ${QM_FILES}
)

target_sources(${module_name} PRIVATE
    ../interfaces/qlcioplugin.cpp ../interfaces/qlcioplugin.h
    configurehid.cpp configurehid.h configurehid.ui
    hiddevice.cpp hiddevice.h
    hiddmxdevice.cpp hiddmxdevice.h
    hidjsdevice.cpp hidjsdevice.h
    ${module_name}.cpp ${module_name}.h
)
target_include_directories(${module_name} PRIVATE
    ../interfaces
)

target_link_libraries(${module_name} PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
)

if(UNIX AND NOT APPLE)
    target_sources(${module_name} PRIVATE
        linux/hidapi.cpp
        linux/hidlinuxjoystick.cpp linux/hidlinuxjoystick.h
    )

    target_include_directories(${module_name} PRIVATE
        linux
    )
endif()

if(APPLE)
    target_sources(${module_name} PRIVATE
        macx/hidapi.cpp
        macx/hidosxjoystick.cpp macx/hidosxjoystick.h
    )
    
    target_include_directories(${module_name} PRIVATE
        macx
    )

    target_link_libraries(${module_name} PRIVATE
        "-framework CoreFoundation"
        "-framework IOKit"
    )
endif()

if(WIN32)
    target_sources(${module_name} PRIVATE
        win32/hidapi.cpp
        win32/hidwindowsjoystick.cpp win32/hidwindowsjoystick.h
    )

    target_link_libraries(${module_name} PRIVATE
        hid
        setupapi
        winmm
    )
endif()

if (UNIX AND NOT APPLE)
   install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/linux/z65-fx5-hid.rules"
           DESTINATION ${UDEVRULESDIR})
   install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/linux/org.qlcplus.QLCPlus.hid.metainfo.xml"
           DESTINATION ${METAINFODIR})
endif()

install(TARGETS ${module_name}
    LIBRARY DESTINATION ${INSTALLROOT}/${PLUGINDIR}
    RUNTIME DESTINATION ${INSTALLROOT}/${PLUGINDIR}
)
