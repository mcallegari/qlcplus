qt_add_library(qlcplusaudio
    audio.cpp audio.h
    audiocapture.cpp audiocapture.h
    audiodecoder.cpp audiodecoder.h
    audioparameters.cpp audioparameters.h
    audioplugincache.cpp audioplugincache.h
    audiorenderer.cpp audiorenderer.h
)
target_include_directories(qlcplusaudio PUBLIC
    ../../../plugins/interfaces
    ../../src
)

target_link_libraries(qlcplusaudio PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Multimedia
)

target_compile_options(qlcplusaudio
    -Werror
)

if(MACOS)
    set_target_properties(qlcplusaudio PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

if(WIN32)
    target_link_libraries(qlcplusaudio PUBLIC
        Qt::Widgets
    )
endif()

if(((QT_VERSION_MAJOR LESS 5)) AND (UNIX AND NOT MACOS))
    target_sources(qlcplusaudio PUBLIC
        audiocapture_alsa.cpp audiocapture_alsa.h
        audiorenderer_alsa.cpp audiorenderer_alsa.h
    )
endif()

if(((QT_VERSION_MAJOR LESS 5)) AND (WIN32))
    target_sources(qlcplusaudio PUBLIC
        audiocapture_wavein.cpp audiocapture_wavein.h
        audiorenderer_waveout.cpp audiorenderer_waveout.h
    )
endif()

if((QT_VERSION_MAJOR LESS 6))
    target_sources(qlcplusaudio PUBLIC
        audiocapture_qt5.cpp audiocapture_qt5.h
        audiorenderer_qt5.cpp audiorenderer_qt5.h
    )
endif()

if(NOT ((QT_VERSION_MAJOR LESS 6)))
    target_sources(qlcplusaudio PUBLIC
        audiocapture_qt6.cpp audiocapture_qt6.h
        audiorenderer_qt6.cpp audiorenderer_qt6.h
    )
endif()

if((((QT_VERSION_MAJOR LESS 5)) AND (MACOS)) AND (system(pkg-config --exists portaudio-2.0)))
    target_sources(qlcplusaudio PUBLIC
        audiocapture_portaudio.cpp audiocapture_portaudio.h
        audiorenderer_portaudio.cpp audiorenderer_portaudio.h
    )

    target_compile_definitions(qlcplusaudio PUBLIC
        HAS_PORTAUDIO
    )
endif()

if((NOT ANDROID AND NOT IOS) AND (system(pkg-config --exists fftw3)))
    target_compile_definitions(qlcplusaudio PUBLIC
        HAS_FFTW3
    )
endif()

if(((NOT ANDROID AND NOT IOS) AND (system(pkg-config --exists fftw3))) AND (MACOS))
    target_link_libraries(qlcplusaudio PUBLIC
        fftw3
    )
endif()

if(UNIX AND NOT ANDROID AND NOT IOS AND NOT MACOS)
    target_link_libraries(qlcplusaudio PUBLIC
        asound
    )
endif()

if(qmlui)
    target_compile_definitions(qlcplusaudio PUBLIC
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(qlcplusaudio
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(qlcplusaudio
        -Wno-unused-local-typedefs
    )
endif()

install(TARGETS qlcplusaudio
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
