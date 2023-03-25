qt_add_executable(velleman_test WIN32 MACOSX_BUNDLE
    ../../interfaces/qlcioplugin.cpp ../../interfaces/qlcioplugin.h
    ../src/velleman_mock.cpp
    velleman_test.cpp velleman_test.h
)
target_include_directories(velleman_test PRIVATE
    ../../interfaces
    ../src
)

target_link_libraries(velleman_test PRIVATE
    # Remove: L../src
    Qt::Core
    Qt::Test
    velleman
)

target_compile_options(velleman_test
    -Werror
)

if(qmlui)
    target_compile_definitions(velleman_test PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(velleman_test
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(velleman_test
        -Wno-unused-local-typedefs
    )
endif()

if(MACOS)
    set_target_properties(velleman_test PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

install(TARGETS velleman_test
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
