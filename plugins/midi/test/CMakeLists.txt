add_executable(midi_test WIN32 MACOSX_BUNDLE
    ../../interfaces/qlcioplugin.cpp ../../interfaces/qlcioplugin.h
    ../src/common/midiprotocol.cpp ../src/common/midiprotocol.h
    midi_test.cpp midi_test.h
)
target_include_directories(midi_test PRIVATE
    ../../interfaces
    ../src/common
)

target_link_libraries(midi_test PRIVATE
    # Remove: L../src
    Qt::Core
    Qt::Test
)

target_compile_options(midi_test
    -Werror
)

if(qmlui)
    target_compile_definitions(midi_test PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(midi_test
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(midi_test
        -Wno-unused-local-typedefs
    )
endif()

if(MACOS)
    set_target_properties(midi_test PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

if(coverage)
    target_link_libraries(midi_test PRIVATE
        gcov
    )

    target_compile_options(midi_test
        -fprofile-arcs
        -ftest-coverage
    )
endif()

install(TARGETS midi_test
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
