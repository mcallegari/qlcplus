qt_add_plugin(e131)
target_sources(e131 PRIVATE
    ../interfaces/qlcioplugin.cpp ../interfaces/qlcioplugin.h
    configuree131.cpp configuree131.h configuree131.ui
    e131controller.cpp e131controller.h
    e131packetizer.cpp e131packetizer.h
    e131plugin.cpp e131plugin.h
)
target_include_directories(e131 PRIVATE
    ../interfaces
)

target_link_libraries(e131 PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::Widgets
)

target_compile_options(e131
    -Werror
)

if(MACOS)
    set_target_properties(e131 PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

if(qmlui)
    target_compile_definitions(e131 PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(e131
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(e131
        -Wno-unused-local-typedefs
    )
endif()

install(TARGETS e131
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
