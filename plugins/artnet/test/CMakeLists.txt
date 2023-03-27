add_executable(artnet_test WIN32 MACOSX_BUNDLE
    ../../interfaces/qlcioplugin.cpp ../../interfaces/qlcioplugin.h
    ../../interfaces/rdmprotocol.cpp ../../interfaces/rdmprotocol.h
    ../src/artnetpacketizer.cpp
    artnet_test.cpp artnet_test.h
)
target_include_directories(artnet_test PRIVATE
    ../../interfaces
    ../src
)

target_link_libraries(artnet_test PRIVATE
    # Remove: L../src
    Qt::Core
    Qt::Network
    Qt::Test
    artnet
)

target_compile_options(artnet_test
    -Werror
)

if(qmlui)
    target_compile_definitions(artnet_test PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(artnet_test
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(artnet_test
        -Wno-unused-local-typedefs
    )
endif()

if(MACOS)
    set_target_properties(artnet_test PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

if(coverage)
    target_link_libraries(artnet_test PRIVATE
        gcov
    )

    target_compile_options(artnet_test
        -fprofile-arcs
        -ftest-coverage
    )
endif()

install(TARGETS artnet_test
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
