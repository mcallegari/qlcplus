set(module_name "qlcplusaudio")

add_library(${module_name}
    STATIC
    audio.cpp audio.h
    audiocapture.cpp audiocapture.h
    audiodecoder.cpp audiodecoder.h
    audioparameters.cpp audioparameters.h
    audioplugincache.cpp audioplugincache.h
    audiorenderer.cpp audiorenderer.h
)
set_property(TARGET ${module_name} PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(${module_name} PUBLIC
    ../../../plugins/interfaces
    ../../src
)

target_link_libraries(${module_name} PUBLIC
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Multimedia
)

if(APPLE)
    set_target_properties(${module_name} PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

if(WIN32)
    target_link_libraries(${module_name} PUBLIC
        Qt${QT_MAJOR_VERSION}::Widgets
    )
endif()

if(((QT_VERSION_MAJOR LESS 5)) AND (UNIX AND NOT APPLE))
    target_sources(${module_name} PRIVATE
        audiocapture_alsa.cpp audiocapture_alsa.h
        audiorenderer_alsa.cpp audiorenderer_alsa.h
    )
endif()

if(((QT_VERSION_MAJOR LESS 5)) AND (WIN32))
    target_sources(${module_name} PRIVATE
        audiocapture_wavein.cpp audiocapture_wavein.h
        audiorenderer_waveout.cpp audiorenderer_waveout.h
    )
endif()

if((QT_VERSION_MAJOR LESS 6))
    target_sources(${module_name} PRIVATE
        audiocapture_qt5.cpp audiocapture_qt5.h
        audiorenderer_qt5.cpp audiorenderer_qt5.h
    )
endif()

if(NOT ((QT_VERSION_MAJOR LESS 6)))
    target_sources(${module_name} PRIVATE
        audiocapture_qt6.cpp audiocapture_qt6.h
        audiorenderer_qt6.cpp audiorenderer_qt6.h
    )
endif()

if((((QT_VERSION_MAJOR LESS 5)) AND (APPLE)))
    if (${PORTAUDIO_2_FOUND})
        target_sources(${module_name} PRIVATE
            audiocapture_portaudio.cpp audiocapture_portaudio.h
            audiorenderer_portaudio.cpp audiorenderer_portaudio.h
        )

        target_compile_definitions(${module_name} PUBLIC
            HAS_PORTAUDIO
        )

        target_include_directories(${module_name} PUBLIC
            ${PORTAUDIO_2_INCLUDE_DIRS}
        )

        target_link_libraries(${module_name} PUBLIC
            ${PORTAUDIO_2_LIBRARIES}
        )
    endif()
endif()

if((NOT ANDROID AND NOT IOS) AND (${FFTW3_FOUND}))
    target_compile_definitions(${module_name} PUBLIC
        HAS_FFTW3
    )

    target_include_directories(${module_name} PUBLIC
        ${FFTW3_INCLUDE_DIRS}
    )
    target_link_libraries(${module_name} PUBLIC
        ${FFTW3_LINK_LIBRARIES}
    )
endif()

if(UNIX AND NOT ANDROID AND NOT IOS AND NOT APPLE)
    target_link_libraries(${module_name} PUBLIC
        asound
    )
endif()
