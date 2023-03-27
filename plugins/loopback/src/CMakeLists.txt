qt_add_plugin(loopback)
target_sources(loopback PRIVATE
    ../../interfaces/qlcioplugin.cpp ../../interfaces/qlcioplugin.h
    loopback.cpp loopback.h
)
target_include_directories(loopback PRIVATE
    ../../interfaces
)

target_link_libraries(loopback PRIVATE
    Qt::Core
    Qt::Gui
)

target_compile_options(loopback
    -Werror
)

if(WIN32)
    target_compile_definitions(loopback PRIVATE
        QLC_EXPORT
    )
endif()

if(MACOS)
    set_target_properties(loopback PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

if(qmlui)
    target_compile_definitions(loopback PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(loopback
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(loopback
        -Wno-unused-local-typedefs
    )
endif()

if(coverage)
    target_link_libraries(loopback PRIVATE
        gcov
    )

    target_compile_options(loopback
        -fprofile-arcs
        -ftest-coverage
    )
endif()

install(TARGETS loopback
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
