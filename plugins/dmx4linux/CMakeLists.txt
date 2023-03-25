qt_add_plugin(dmx4linux)
target_sources(dmx4linux PRIVATE
    ../interfaces/qlcioplugin.cpp ../interfaces/qlcioplugin.h
    dmx4linux.cpp dmx4linux.h
)
target_include_directories(dmx4linux PRIVATE
    ../interfaces
)

target_link_libraries(dmx4linux PRIVATE
    Qt::Core
    Qt::Gui
)

target_compile_options(dmx4linux
    -Werror
)

if(qmlui)
    target_compile_definitions(dmx4linux PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(dmx4linux
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(dmx4linux
        -Wno-unused-local-typedefs
    )
endif()

if(MACOS)
    set_target_properties(dmx4linux PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

install(TARGETS dmx4linux
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
