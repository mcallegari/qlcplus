set(module_name "qlcplusengine")

include(../../variables.cmake)

add_library(${module_name} SHARED
    ../../plugins/interfaces/qlcioplugin.cpp ../../plugins/interfaces/qlcioplugin.h
    avolitesd4parser.cpp avolitesd4parser.h
    bus.cpp bus.h
    channelmodifier.cpp channelmodifier.h
    channelsgroup.cpp channelsgroup.h
    chaser.cpp chaser.h
    chaseraction.h
    chaserrunner.cpp chaserrunner.h
    chaserstep.cpp chaserstep.h
    collection.cpp collection.h
    cue.cpp cue.h
    cuestack.cpp cuestack.h
    dmxdumpfactoryproperties.cpp dmxdumpfactoryproperties.h
    dmxsource.h
    doc.cpp doc.h
    efx.cpp efx.h
    efxfixture.cpp efxfixture.h
    fadechannel.cpp fadechannel.h
    fixture.cpp fixture.h
    fixturegroup.cpp fixturegroup.h
    function.cpp function.h
    genericdmxsource.cpp genericdmxsource.h
    genericfader.cpp genericfader.h
    gradient.cpp gradient.h
    grandmaster.cpp grandmaster.h
    grouphead.cpp grouphead.h
    inputoutputmap.cpp inputoutputmap.h
    inputpatch.cpp inputpatch.h
    ioplugincache.cpp ioplugincache.h
    keypadparser.cpp keypadparser.h
    mastertimer.cpp mastertimer.h
    monitorproperties.cpp monitorproperties.h
    outputpatch.cpp outputpatch.h
    qlccapability.cpp qlccapability.h
    qlcchannel.cpp qlcchannel.h
    qlcclipboard.cpp qlcclipboard.h
    qlcfile.cpp qlcfile.h
    qlcfixturedef.cpp qlcfixturedef.h
    qlcfixturedefcache.cpp qlcfixturedefcache.h
    qlcfixturehead.cpp qlcfixturehead.h
    qlcfixturemode.cpp qlcfixturemode.h
    qlci18n.cpp qlci18n.h
    qlcinputchannel.cpp qlcinputchannel.h
    qlcinputfeedback.cpp qlcinputfeedback.h
    qlcinputprofile.cpp qlcinputprofile.h
    qlcinputsource.cpp qlcinputsource.h
    qlcmodifierscache.cpp qlcmodifierscache.h
    qlcpalette.cpp qlcpalette.h
    qlcphysical.cpp qlcphysical.h
    qlcpoint.cpp qlcpoint.h
    rgbalgorithm.cpp rgbalgorithm.h
    rgbaudio.cpp rgbaudio.h
    rgbimage.cpp rgbimage.h
    rgbmatrix.cpp rgbmatrix.h
    rgbplain.cpp rgbplain.h
    rgbscriptproperty.h
    rgbscriptscache.cpp rgbscriptscache.h
    rgbtext.cpp rgbtext.h
    scene.cpp scene.h
    scenevalue.cpp scenevalue.h
    scriptwrapper.h
    sequence.cpp sequence.h
    show.cpp show.h
    showfunction.cpp showfunction.h
    showrunner.cpp showrunner.h
    track.cpp track.h
    universe.cpp universe.h
    utils.h
    video.cpp video.h
)
target_include_directories(${module_name} PUBLIC
    ../../plugins/interfaces
    ../audio/src
)

target_link_libraries(${module_name} PUBLIC
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Multimedia
)

target_link_libraries(${module_name} PRIVATE
    qlcplusaudio
)

if(NOT ANDROID AND NOT IOS)
    target_include_directories(${module_name} PUBLIC
        ../../hotplugmonitor/src
    )

    target_link_libraries(${module_name} PUBLIC
        hotplugmonitor
    )

    target_link_directories(${module_name} PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}/../../hotplugmonitor/src
    )
endif()

if(UNIX AND NOT ANDROID AND NOT IOS AND NOT APPLE)
    target_link_libraries(${module_name} PUBLIC
        asound
    )
endif()

if(WIN32)
    target_sources(${module_name} PRIVATE
        mastertimer-win32.cpp mastertimer-win32.h
    )

    target_include_directories(${module_name} PUBLIC
        /
    )

    target_link_libraries(${module_name} PUBLIC
        Qt${QT_MAJOR_VERSION}::Widgets
        winmm
    )
endif()

if((NOT ANDROID AND NOT IOS) AND (iokit))
    target_link_libraries(${module_name} PUBLIC
        "-framework CoreFoundation"
        "-framework IOKit"
    )
endif()

if(APPLE)
    set_target_properties(${module_name} PROPERTIES
        MACOSX_BUNDLE FALSE
    )
    target_link_libraries(${module_name} PUBLIC
        "-framework AudioToolbox"
        "-framework CoreAudio"
        "-framework CoreFoundation"
    )
endif()

if(qmlui OR (QT_VERSION_MAJOR GREATER 5))
    target_sources(${module_name} PRIVATE
        rgbscriptv4.cpp rgbscriptv4.h
        scriptrunner.cpp scriptrunner.h
        scriptv4.cpp scriptv4.h
    )

    target_link_libraries(${module_name} PUBLIC
        Qt${QT_MAJOR_VERSION}::Qml
    )
endif()

if(NOT (qmlui OR (QT_VERSION_MAJOR GREATER 5)))
    target_sources(${module_name} PRIVATE
        rgbscript.cpp rgbscript.h
        script.cpp script.h
    )

    target_link_libraries(${module_name} PUBLIC
        Qt${QT_MAJOR_VERSION}::Script
    )
endif()

if(UNIX)
    target_sources(${module_name} PRIVATE
        mastertimer-unix.cpp mastertimer-unix.h
    )
endif()

if((NOT ANDROID AND NOT IOS) AND (${FFTW3_FOUND}))
    target_compile_definitions(${module_name} PUBLIC
        HAS_FFTW3
    )
endif()

if(((NOT ANDROID AND NOT IOS) AND (${FFTW3_FOUND})))
    target_link_directories(${module_name} PUBLIC
        ${FFTW3_LIBRARY_DIRS}
    )
    target_link_libraries(${module_name} PUBLIC
        ${FFTW3_LIBRARIES}
    )
endif()

set(CONFIGFILE ${CMAKE_SOURCE_DIR}/engine/src/qlcconfig.h)
set(LITERAL_HASH "#")

if(WIN32 OR APPLE OR appimage)
    configure_file(qlcconfig.h.noroot.in ${CONFIGFILE})
elseif(UNIX OR ANDROID OR IOS)
    configure_file(qlcconfig.h.in ${CONFIGFILE})
endif()

set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${CONFIGFILE})

install(TARGETS ${module_name}
    LIBRARY DESTINATION ${INSTALLROOT}/${LIBSDIR}
    RUNTIME DESTINATION ${INSTALLROOT}/${LIBSDIR}
)
