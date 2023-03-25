qt_add_plugin(os2l)
target_sources(os2l PRIVATE
    ../interfaces/qlcioplugin.cpp ../interfaces/qlcioplugin.h
    os2lconfiguration.cpp os2lconfiguration.h os2lconfiguration.ui
    os2lplugin.cpp os2lplugin.h
)
target_include_directories(os2l PRIVATE
    ../interfaces
)

target_link_libraries(os2l PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::Widgets
)

target_compile_options(os2l
    -Werror
)

if(qmlui)
    target_compile_definitions(os2l PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(os2l
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(os2l
        -Wno-unused-local-typedefs
    )
endif()

if(MACOS)
    set_target_properties(os2l PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

install(TARGETS os2l
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
