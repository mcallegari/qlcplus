qt_add_plugin(udmx)
target_sources(udmx PRIVATE
    ../../interfaces/qlcioplugin.cpp ../../interfaces/qlcioplugin.h
    udmx.cpp udmx.h
    udmxdevice.cpp udmxdevice.h
)
target_include_directories(udmx PRIVATE
    ../../interfaces
)

target_link_libraries(udmx PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
)

target_compile_options(udmx
    -Werror
)

if(MACOS)
    set_target_properties(udmx PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

if(WIN32)
    target_sources(udmx PUBLIC
        libusb_dyn.c libusb_dyn.h
    )
endif()

if(qmlui)
    target_compile_definitions(udmx PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(udmx
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(udmx
        -Wno-unused-local-typedefs
    )
endif()

install(TARGETS udmx
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
