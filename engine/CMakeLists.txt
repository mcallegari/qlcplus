find_package(Portaudio REQUIRED)
find_package(FFTW)
find_package(Mad)
find_package(LibSndFile)

set(GOBODIR Gobos)

configure_file(qlcconfig.h.in qlcconfig.h)

set (audioengine_src 
        audio/src/audio.cpp 
        audio/src/audiodecoder.cpp 
        audio/src/audiorenderer.cpp
        audio/src/audioparameters.cpp 
        audio/src/audiocapture.cpp 
        audio/src/audioplugincache.cpp 
        audio/src/audiorenderer_portaudio.cpp
        audio/src/audiocapture_portaudio.cpp
)

include_directories(
    ${CMAKE_SOURCE_DIR}/hotplugmonitor/src
    ${CMAKE_SOURCE_DIR}/engine/audio/src/
    ${CMAKE_SOURCE_DIR}/engine/src/
    ${CMAKE_SOURCE_DIR}/plugins/
    ${CMAKE_SOURCE_DIR}/plugins/interfaces/
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Script_INCLUDE_DIRS}
    ${Qt5Multimedia_INCLUDE_DIRS}
    ${Qt5Qml_INCLUDE_DIRS}
    ${Qt5Test_INCLUDE_DIRS}
    ${FFTW_INCLUDE_DIRS} OPTIONAL
    ${MAD_INCLUDE_DIR} OPTIONAL
    ${LIBSNDFILE_INCLUDE_DIRS} OPTIONAL
)

set(audioengine_link
    Qt5::Core
    ${PORTAUDIO_LIBRARIES}
)

if(${FFTW_FOUND})
    set ( audioengine_link
        ${audioengine_link}
        ${FFTW_LIBRARIES}
    )
    add_definitions(-DHAS_FFTW3)
endif(${FFTW_FOUND})

if(${MAD_FOUND})
    set (madplugin_src 
        audio/plugins/mad/audiodecoder_mad.cpp
    )
    add_library(madplugin SHARED ${madplugin_src})
    target_link_libraries(madplugin ${MAD_LIBRARIES})
    install(TARGETS madplugin DESTINATION ${AUDIOPLUGINDIR})
endif(${MAD_FOUND})

if(${LIBSNDFILE_FOUND})
    set (sndfileplugin_src 
        audio/plugins/sndfile/audiodecoder_sndfile.cpp
    )
    add_library(sndfileplugin SHARED ${sndfileplugin_src})
    install(TARGETS sndfileplugin DESTINATION ${AUDIOPLUGINDIR})
    target_link_libraries(sndfileplugin ${LIBSNDFILE_LIBRARIES})
endif(${LIBSNDFILE_FOUND})

add_library(qlcplusaudio STATIC ${audioengine_src})

target_link_libraries(qlcplusaudio ${audioengine_link})

set(qlcplusengine_src
           src/avolitesd4parser.cpp 
           src/qlccapability.cpp 
           src/qlcchannel.cpp 
           src/qlcfile.cpp 
           src/qlcfixturedef.cpp 
           src/qlcfixturedefcache.cpp 
           src/qlcfixturehead.cpp 
           src/qlcfixturemode.cpp 
           src/qlci18n.cpp 
           src/qlcinputchannel.cpp 
           src/qlcinputprofile.cpp 
           src/qlcinputsource.cpp 
           src/qlcmodifierscache.cpp 
           src/qlcpalette.cpp 
           src/qlcphysical.cpp
           src/video.cpp
           #engine
           src/bus.cpp 
           src/channelsgroup.cpp 
           src/channelmodifier.cpp 
           src/chaser.cpp
           src/chaserrunner.cpp
           src/chaserstep.cpp
           src/collection.cpp
           src/cue.cpp
           src/cuestack.cpp
           src/doc.cpp
           src/dmxdumpfactoryproperties.cpp
           src/efx.cpp
           src/efxfixture.cpp
           src/fadechannel.cpp
           src/fixture.cpp
           src/fixturegroup.cpp
           src/function.cpp
           src/genericdmxsource.cpp
           src/genericfader.cpp
           src/gradient.cpp
           src/grandmaster.cpp
           src/grouphead.cpp
           src/inputoutputmap.cpp
           src/inputpatch.cpp
           src/ioplugincache.cpp
           src/mastertimer.cpp
           src/monitorproperties.cpp
           src/outputpatch.cpp
           src/qlcclipboard.cpp
           src/qlcpoint.cpp
           src/rgbalgorithm.cpp
           src/rgbaudio.cpp
           src/rgbmatrix.cpp
           src/rgbimage.cpp
           src/rgbplain.cpp
           src/rgbscriptscache.cpp
           src/rgbtext.cpp
           src/scene.cpp
           src/scenevalue.cpp
           src/sequence.cpp
           src/show.cpp
           src/showfunction.cpp
           src/showrunner.cpp
           src/track.cpp
           src/universe.cpp
           src/rgbscript.cpp 
           src/scriptrunner.cpp 
           src/script.cpp
)

IF (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(qlcplusengine_src
        ${qlcplusengine_src}
        src/mastertimer-win32.cpp
    )
ELSE()
    set(qlcplusengine_src
        ${qlcplusengine_src}
        src/mastertimer-unix.cpp
    )
ENDIF()

set(qlcplusengine_link
    Qt5::Qml
    Qt5::Script
    Qt5::Multimedia
    Qt5::Widgets
    Qt5::Xml
    qlcplusaudio
    hotplugmonitor
)

add_library(qlcplusengine SHARED ${qlcplusengine_src})
target_link_libraries(qlcplusengine ${qlcplusengine_link})

install(TARGETS qlcplusengine DESTINATION "${LIBSDIR}")

#build tests
add_definitions(-DUSE_CTEST)
set(CMAKE_AUTOMOC ON)

include_directories(
    ${INCLUDE_DIRECTORIES}
    ${CMAKE_SOURCE_DIR}/engine/test/
    ${CMAKE_SOURCE_DIR}/engine/test/common
    ${CMAKE_SOURCE_DIR}/engine/test/mastertimer
    ${CMAKE_SOURCE_DIR}/engine/test/bus
    ${CMAKE_SOURCE_DIR}/engine/test/chaser
    ${CMAKE_SOURCE_DIR}/engine/test/chaserrunner
    ${CMAKE_SOURCE_DIR}/engine/test/chaserstep
    ${CMAKE_SOURCE_DIR}/engine/test/collection
    ${CMAKE_SOURCE_DIR}/engine/test/cue
    ${CMAKE_SOURCE_DIR}/engine/test/cuestack
    ${CMAKE_SOURCE_DIR}/engine/test/doc
    ${CMAKE_SOURCE_DIR}/plugins/interfaces
)
#bus test
add_executable(bus_test
    test/bus/bus_test.cpp
)
target_link_libraries(bus_test PRIVATE Qt5::Test qlcplusengine)
add_test(bus_test bus_test)

#chaser test
add_executable(chaser_test 
    test/chaser/chaser_test.cpp
    test/mastertimer/mastertimer_stub.cpp
)
target_link_libraries(chaser_test PRIVATE Qt5::Test qlcplusengine)
add_test(chaser_test chaser_test)

#chaserrunner test
add_executable(chaserrunner_test 
    test/chaserrunner/chaserrunner_test.cpp 
    test/mastertimer/mastertimer_stub.cpp
)
target_link_libraries(chaserrunner_test PRIVATE Qt5::Test qlcplusengine)
add_test(chaserrunner_test chaserrunner_test)

#chaserstep test
add_executable(chaserstep_test 
    test/chaserstep/chaserstep_test.cpp 
)
target_link_libraries(chaserstep_test PRIVATE Qt5::Test qlcplusengine)
add_test(chaserstep_test chaserstep_test)

#collection test
add_executable(collection_test 
    test/collection/collection_test.cpp 
    test/mastertimer/mastertimer_stub.cpp
)
target_link_libraries(collection_test PRIVATE Qt5::Test qlcplusengine)
add_test(collection_test collection_test)

#cue test
add_executable(cue_test 
    test/cue/cue_test.cpp 
)
target_link_libraries(cue_test PRIVATE Qt5::Test qlcplusengine)
add_test(cue_test cue_test)

#cuestack test
add_executable(cuestack_test 
    test/cuestack/cuestack_test.cpp 
)
target_link_libraries(cuestack_test PRIVATE Qt5::Test qlcplusengine)
add_test(cuestack_test cuestack_test)

#doctest
add_executable(doc_test 
    test/doc/doc_test.cpp 
)
target_link_libraries(doc_test PRIVATE Qt5::Test qlcplusengine)
add_test(doc_test doc_test)
