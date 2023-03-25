qt_add_plugin(madplugin)
target_sources(madplugin PRIVATE
    ../../src/audiodecoder.cpp ../../src/audiodecoder.h
    ../../src/audioparameters.cpp ../../src/audioparameters.h
    audiodecoder_mad.cpp audiodecoder_mad.h
)
target_include_directories(madplugin PRIVATE
    ../../src
)

target_link_libraries(madplugin PRIVATE
    Qt::Core
    Qt::Gui
)

target_compile_options(madplugin
    -Werror
)

if(MACOS)
    set_target_properties(madplugin PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

if(qmlui)
    target_compile_definitions(madplugin PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(madplugin
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(madplugin
        -Wno-unused-local-typedefs
    )
endif()

install(TARGETS madplugin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
