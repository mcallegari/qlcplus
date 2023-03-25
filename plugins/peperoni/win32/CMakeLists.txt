qt_add_plugin(peperoni)
target_sources(peperoni PRIVATE
    ../../interfaces/qlcioplugin.cpp ../../interfaces/qlcioplugin.h
    peperoni.cpp peperoni.h
    peperoni/usbdmx-dynamic.cpp peperoni/usbdmx-dynamic.h
    peperonidevice.cpp peperonidevice.h
)
target_include_directories(peperoni PRIVATE
    ../../interfaces
    peperoni
)

target_link_libraries(peperoni PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
)

target_compile_options(peperoni
    -Werror
)

if(qmlui)
    target_compile_definitions(peperoni PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(peperoni
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(peperoni
        -Wno-unused-local-typedefs
    )
endif()

if(MACOS)
    set_target_properties(peperoni PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

install(TARGETS peperoni
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
