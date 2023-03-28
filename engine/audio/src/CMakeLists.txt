include(../../../variables.cmake)
find_package(PkgConfig REQUIRED)

add_library(qlcplusaudio
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
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Multimedia
)

if(MACOS)
    set_target_properties(qlcplusaudio PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

if(WIN32)
    target_link_libraries(qlcplusaudio PUBLIC
        Qt${QT_MAJOR_VERSION}::Widgets
    )
endif()

if(((QT_VERSION_MAJOR LESS 5)) AND (UNIX AND NOT MACOS))
    target_sources(qlcplusaudio PRIVATE
        audiocapture_alsa.cpp audiocapture_alsa.h
        audiorenderer_alsa.cpp audiorenderer_alsa.h
    )
endif()

if(((QT_VERSION_MAJOR LESS 5)) AND (WIN32))
    target_sources(qlcplusaudio PRIVATE
        audiocapture_wavein.cpp audiocapture_wavein.h
        audiorenderer_waveout.cpp audiorenderer_waveout.h
    )
endif()

if((QT_VERSION_MAJOR LESS 6))
    target_sources(qlcplusaudio PRIVATE
        audiocapture_qt5.cpp audiocapture_qt5.h
        audiorenderer_qt5.cpp audiorenderer_qt5.h
    )
endif()

if(NOT ((QT_VERSION_MAJOR LESS 6)))
    target_sources(qlcplusaudio PRIVATE
        audiocapture_qt6.cpp audiocapture_qt6.h
        audiorenderer_qt6.cpp audiorenderer_qt6.h
    )
endif()

pkg_check_modules(PORTAUDIO_2 IMPORTED_TARGET portaudio-2.0)

if((((QT_VERSION_MAJOR LESS 5)) AND (MACOS)) AND (${PORTAUDIO_2_FOUND}))
    target_sources(qlcplusaudio PRIVATE
        audiocapture_portaudio.cpp audiocapture_portaudio.h
        audiorenderer_portaudio.cpp audiorenderer_portaudio.h
    )

    target_compile_definitions(qlcplusaudio PUBLIC
        HAS_PORTAUDIO
    )
endif()

pkg_check_modules(FFTW3 IMPORTED_TARGET fftw3)

if((NOT ANDROID AND NOT IOS) AND (${FFTW3_FOUND}))
    target_compile_definitions(qlcplusaudio PUBLIC
        HAS_FFTW3
    )
endif()

if(((NOT ANDROID AND NOT IOS) AND (${FFTW3_FOUND})) AND (MACOS))
    target_link_libraries(qlcplusaudio PUBLIC
        fftw3
    )
endif()

if(UNIX AND NOT ANDROID AND NOT IOS AND NOT MACOS)
    target_link_libraries(qlcplusaudio PUBLIC
        asound
    )
endif()

install(TARGETS qlcplusaudio
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
