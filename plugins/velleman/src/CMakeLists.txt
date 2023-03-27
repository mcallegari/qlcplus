qt_add_plugin(velleman)
target_sources(velleman PRIVATE
    ../../interfaces/qlcioplugin.cpp ../../interfaces/qlcioplugin.h
    velleman.cpp velleman.h
)
target_include_directories(velleman PRIVATE
    ../../interfaces
)

target_link_libraries(velleman PRIVATE
    Qt::Core
    Qt::Gui
)

target_compile_options(velleman
    -Werror
)

if(WIN32)
    target_compile_definitions(velleman PRIVATE
        QLC_EXPORT
    )

    target_link_libraries(velleman PRIVATE
        # Remove: LC:/Qt/K8062D
        C:/Qt/K8062D/K8062D.a
        K8062D
    )
endif()

if(UNIX)
    target_sources(velleman PUBLIC
        velleman_mock.cpp
    )
endif()

if(qmlui)
    target_compile_definitions(velleman PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(velleman
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(velleman
        -Wno-unused-local-typedefs
    )
endif()

if(MACOS)
    set_target_properties(velleman PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

if(coverage)
    target_link_libraries(velleman PRIVATE
        gcov
    )

    target_compile_options(velleman
        -fprofile-arcs
        -ftest-coverage
    )
endif()

install(TARGETS velleman
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
