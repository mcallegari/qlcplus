qt_add_plugin(sndfileplugin)
target_sources(sndfileplugin PRIVATE
    ../../src/audiodecoder.cpp ../../src/audiodecoder.h
    ../../src/audioparameters.cpp ../../src/audioparameters.h
    audiodecoder_sndfile.cpp audiodecoder_sndfile.h
)
target_include_directories(sndfileplugin PRIVATE
    ../../src
)

target_link_libraries(sndfileplugin PRIVATE
    Qt::Core
    Qt::Gui
)

target_compile_options(sndfileplugin
    -Werror
)

if(MACOS)
    set_target_properties(sndfileplugin PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

if(qmlui)
    target_compile_definitions(sndfileplugin PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(sndfileplugin
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(sndfileplugin
        -Wno-unused-local-typedefs
    )
endif()

install(TARGETS sndfileplugin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
