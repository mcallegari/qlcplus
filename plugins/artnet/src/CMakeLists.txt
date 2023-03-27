qt_add_plugin(artnet)
target_sources(artnet PRIVATE
    ../../interfaces/qlcioplugin.cpp ../../interfaces/qlcioplugin.h
    ../../interfaces/rdmprotocol.cpp ../../interfaces/rdmprotocol.h
    artnetcontroller.cpp artnetcontroller.h
    artnetpacketizer.cpp artnetpacketizer.h
    artnetplugin.cpp artnetplugin.h
    configureartnet.cpp configureartnet.h configureartnet.ui
)
target_include_directories(artnet PRIVATE
    ../../interfaces
)

target_link_libraries(artnet PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::Widgets
)

target_compile_options(artnet
    -Werror
)

if(MACOS)
    set_target_properties(artnet PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

if(qmlui)
    target_compile_definitions(artnet PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(artnet
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(artnet
        -Wno-unused-local-typedefs
    )
endif()

if(coverage)
    target_link_libraries(artnet PRIVATE
        gcov
    )

    target_compile_options(artnet
        -fprofile-arcs
        -ftest-coverage
    )
endif()

install(TARGETS artnet
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
