qt_add_plugin(enttecwing)
target_sources(enttecwing PRIVATE
    ../../interfaces/qlcioplugin.cpp ../../interfaces/qlcioplugin.h
    enttecwing.cpp enttecwing.h
    playbackwing.cpp playbackwing.h
    programwing.cpp programwing.h
    shortcutwing.cpp shortcutwing.h
    wing.cpp wing.h
)
target_include_directories(enttecwing PRIVATE
    ../../interfaces
)

target_link_libraries(enttecwing PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::Widgets
)

target_compile_options(enttecwing
    -Werror
)

if(WIN32)
    target_compile_definitions(enttecwing PRIVATE
        QLC_EXPORT
    )
endif()

if(MACOS)
    set_target_properties(enttecwing PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

if(qmlui)
    target_compile_definitions(enttecwing PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(enttecwing
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(enttecwing
        -Wno-unused-local-typedefs
    )
endif()

if(CONFIG(coverage))
    target_link_libraries(enttecwing PRIVATE
        gcov
    )

    target_compile_options(enttecwing
        -fprofile-arcs
        -ftest-coverage
    )
endif()

install(TARGETS enttecwing
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
