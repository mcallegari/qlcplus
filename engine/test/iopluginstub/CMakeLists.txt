qt_add_plugin(iopluginstub)
target_sources(iopluginstub PRIVATE
    ../../../plugins/interfaces/qlcioplugin.cpp ../../../plugins/interfaces/qlcioplugin.h
    iopluginstub.cpp iopluginstub.h
)
target_include_directories(iopluginstub PRIVATE
    ../../../plugins/interfaces
)

target_link_libraries(iopluginstub PRIVATE
    Qt::Core
    Qt::Gui
)

target_compile_options(iopluginstub
    -Werror
)

if(qmlui)
    target_compile_definitions(iopluginstub PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(iopluginstub
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(iopluginstub
        -Wno-unused-local-typedefs
    )
endif()

if(MACOS)
    set_target_properties(iopluginstub PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

install(TARGETS iopluginstub
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
