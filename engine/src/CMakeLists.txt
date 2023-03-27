add_library(qlcplusengine
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
target_include_directories(qlcplusengine PUBLIC
    ../../plugins/interfaces
    ../audio/src
)

target_link_libraries(qlcplusengine PUBLIC
    # Remove: L../audio/src
    Qt::Core
    Qt::Gui
    Qt::Multimedia
    qlcplusaudio
)

target_compile_options(qlcplusengine
    -Werror
)

if(NOT ANDROID AND NOT IOS)
    target_include_directories(qlcplusengine PUBLIC
        ../../hotplugmonitor/src
    )

    target_link_libraries(qlcplusengine PUBLIC
        # Remove: L../../hotplugmonitor/src
        hotplugmonitor
    )
endif()

if((NOT ANDROID AND NOT IOS) AND (CONFIG(coverage)))
    target_link_libraries(qlcplusengine PUBLIC
        gcov
    )

    target_compile_options(qlcplusengine
        -fprofile-arcs
        -ftest-coverage
    )
endif()

if(UNIX AND NOT ANDROID AND NOT IOS AND NOT MACOS)
    target_link_libraries(qlcplusengine PUBLIC
        asound
    )
endif()

if(WIN32)
    target_sources(qlcplusengine PUBLIC
        mastertimer-win32.cpp mastertimer-win32.h
    )

    target_include_directories(qlcplusengine PUBLIC
        /
    )

    target_link_libraries(qlcplusengine PUBLIC
        Qt::Widgets
        winmm
    )
endif()

if((NOT ANDROID AND NOT IOS) AND (CONFIG(iokit)))
    target_link_libraries(qlcplusengine PUBLIC
        "-framework CoreFoundation"
        "-framework IOKit"
    )
endif()

if(MACOS)
    set_target_properties(qlcplusengine PROPERTIES
        MACOSX_BUNDLE FALSE
    )
    target_link_libraries(qlcplusengine PUBLIC
        "-framework AudioToolbox"
        "-framework CoreAudio"
        "-framework CoreFoundation"
    )
endif()

if(qmlui OR (QT_VERSION_MAJOR GREATER 5))
    target_sources(qlcplusengine PUBLIC
        rgbscriptv4.cpp rgbscriptv4.h
        scriptrunner.cpp scriptrunner.h
        scriptv4.cpp scriptv4.h
    )

    target_link_libraries(qlcplusengine PUBLIC
        Qt::Qml
    )
endif()

if(NOT (qmlui OR (QT_VERSION_MAJOR GREATER 5)))
    target_sources(qlcplusengine PUBLIC
        rgbscript.cpp rgbscript.h
        script.cpp script.h
    )

    target_link_libraries(qlcplusengine PUBLIC
        Qt::Script
    )
endif()

if(UNIX)
    target_sources(qlcplusengine PUBLIC
        mastertimer-unix.cpp mastertimer-unix.h
    )
endif()

if((NOT ANDROID AND NOT IOS) AND (system(pkg-config --exists fftw3)))
    target_compile_definitions(qlcplusengine PUBLIC
        HAS_FFTW3
    )
endif()

if(((NOT ANDROID AND NOT IOS) AND (system(pkg-config --exists fftw3))) AND (MACOS))
    target_link_libraries(qlcplusengine PUBLIC
        fftw3
    )
endif()

if(qmlui)
    target_compile_definitions(qlcplusengine PUBLIC
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(qlcplusengine
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(qlcplusengine
        -Wno-unused-local-typedefs
    )
endif()

install(TARGETS qlcplusengine
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
