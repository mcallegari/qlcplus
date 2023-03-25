qt_add_plugin(spi)
target_sources(spi PRIVATE
    ../interfaces/qlcioplugin.cpp ../interfaces/qlcioplugin.h
    spiconfiguration.cpp spiconfiguration.h spiconfiguration.ui
    spioutthread.cpp spioutthread.h
    spiplugin.cpp spiplugin.h
)
target_include_directories(spi PRIVATE
    $SYSROOT/usr/include
    ../interfaces
)

target_link_libraries(spi PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
)

target_compile_options(spi
    -Werror
)

if(qmlui)
    target_compile_definitions(spi PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(spi
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(spi
        -Wno-unused-local-typedefs
    )
endif()

if(MACOS)
    set_target_properties(spi PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

install(TARGETS spi
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
