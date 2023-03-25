qt_add_plugin(osc)
target_sources(osc PRIVATE
    ../interfaces/qlcioplugin.cpp ../interfaces/qlcioplugin.h
    configureosc.cpp configureosc.h configureosc.ui
    osccontroller.cpp osccontroller.h
    oscpacketizer.cpp oscpacketizer.h
    oscplugin.cpp oscplugin.h
)
target_include_directories(osc PRIVATE
    ../interfaces
)

target_link_libraries(osc PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::Widgets
)

target_compile_options(osc
    -Werror
)

if(MACOS)
    set_target_properties(osc PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

if(qmlui)
    target_compile_definitions(osc PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(osc
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(osc
        -Wno-unused-local-typedefs
    )
endif()

install(TARGETS osc
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
