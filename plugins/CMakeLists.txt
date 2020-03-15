include_directories(
    ${CMAKE_BINARY_DIR}/engine
    ${CMAKE_SOURCE_DIR}/engine/src
    interfaces
    artnet/src/
    enttecwing/src
    dmxusb/src/
    midi/src/common
    hid/
    hid/linux OPTIONAL
    hid/macx  OPTIONAL
    hid/win32 OPTIONAL
    midi/alsa OPTIONAL
    midi/win32 OPTIONAL
    midi/macx OPTIONAL
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Test_INCLUDE_DIRS}
)

#arnet plugin
set(artnet_src
    interfaces/qlcioplugin.cpp
    artnet/src/artnetcontroller.cpp
    artnet/src/artnetpacketizer.cpp
    artnet/src/configureartnet.cpp
    artnet/src/artnetplugin.cpp
)

add_library(artnet SHARED ${artnet_src})

target_link_libraries(artnet Qt5::Core Qt5::Widgets Qt5::Network)

add_executable(artnet_test artnet/test/artnet_test.cpp)

target_link_libraries(artnet_test artnet Qt5::Test)

add_test(artnet_test artnet_test)

install(TARGETS artnet DESTINATION ${PLUGINDIR})
install(FILES artnet/src/org.qlcplus.QLCPlus.artnet.metainfo.xml DESTINATION ${METAINFODIR})

#dmx4linux plugin
IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_library(dmx4linux SHARED interfaces/qlcioplugin.cpp dmx4linux/dmx4linux.cpp)

    target_link_libraries(dmx4linux Qt5::Core )

    install(TARGETS dmx4linux DESTINATION ${PLUGINDIR})
    install(FILES dmx4linux/org.qlcplus.QLCPlus.dmx4linux.metainfo.xml DESTINATION ${METAINFODIR})
ENDIF()

#dmxusb plugin

set(dmxusb_src
    midi/src/common/midiprotocol.cpp
    interfaces/qlcioplugin.cpp
    dmxusb/src/dmxinterface.cpp
    dmxusb/src/dmxusbconfig.cpp
    dmxusb/src/dmxusb.cpp
    dmxusb/src/dmxusbopenrx.cpp
    dmxusb/src/dmxusbwidget.cpp
    dmxusb/src/enttecdmxusbopen.cpp
    dmxusb/src/enttecdmxusbpro.cpp
    dmxusb/src/stageprofi.cpp
    dmxusb/src/vinceusbdmx512.cpp
)

if(FTD2XX_FOUND)
    add_definitions(-DFTD2XX)
    include_directories(
        ${FTD2XX_INCLUDE_DIR} 
        ${LIBUSB_1_INCLUDE_DIRS}
    )
    message(STATUS "Building with libFTD2XX support")
    list(APPEND dmxusb_src
        dmxusb/src/ftd2xx-interface.cpp
    )
    list(APPEND dmxusb_link ${FTD2XX_LIBRARY} ${LIBUSB_1_LIBRARIES})
elseif(${FTDI_FOUND})
    message(STATUS "Building with libFTDI1 support")
    add_definitions(-DLIBFTDI1)
    include_directories(
        ${FTDI_INCLUDE_DIRS} 
        ${LIBUSB_1_INCLUDE_DIRS}
    )
    list(APPEND dmxusb_src 
        dmxusb/src/libftdi-interface.cpp
    )
    IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        install(FILES dmxusb/src/z65-dmxusb.rules DESTINATION ${UDEVRULESDIR})
    ENDIF()
    list(APPEND dmxusb_link ${FTDI_LIBRARIES} ${LIBUSB_1_LIBRARIES})
else()
    message(STATUS "Building with QtSerial support")
    add_definitions(-DQTSERIAL)
    include_directories(
        ${Qt5SerialPort_INCLUDE_DIRS}
    )
    list(APPEND dmxusb_src dmxusb/src/qtserial-interface.cpp)
    list(APPEND dmxusb_link Qt5::SerialPort)
endif()

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    list(APPEND dmxusb_src
        dmxusb/src/nanodmx.cpp
        dmxusb/src/euroliteusbdmxpro.cpp
    )
endif()

add_library(dmxusb SHARED ${dmxusb_src})

target_link_libraries(dmxusb
    Qt5::Core
    Qt5::Widgets
    ${dmxusb_link}
)

install(TARGETS dmxusb DESTINATION ${PLUGINDIR})
install(FILES dmxusb/src/org.qlcplus.QLCPlus.dmxusb.metainfo.xml DESTINATION ${METAINFODIR})

#dummy plugin

set(dummy_src
    interfaces/qlcioplugin.cpp
    dummy/dummyplugin.cpp 
    dummy/dummyconfiguration.cpp
)

add_library(dummy SHARED ${dummy_src})

target_link_libraries(dummy
    Qt5::Core
    Qt5::Widgets
)

install(TARGETS dummy DESTINATION ${PLUGINDIR})

#E1.31 plugin

set(E1.31_src
    interfaces/qlcioplugin.cpp
    E1.31/e131packetizer.cpp 
    E1.31/e131controller.cpp 
    E1.31/e131plugin.cpp 
    E1.31/configuree131.cpp
)

add_library(E1.31 SHARED ${E1.31_src})
install(TARGETS E1.31 DESTINATION ${PLUGINDIR})
install(FILES E1.31/org.qlcplus.QLCPlus.e131.metainfo.xml DESTINATION ${METAINFODIR})

#enttecwing plugin
set(enttecwing_src
    interfaces/qlcioplugin.cpp
    enttecwing/src/enttecwing.cpp
    enttecwing/src/playbackwing.cpp
    enttecwing/src/programwing.cpp
    enttecwing/src/shortcutwing.cpp
    enttecwing/src/wing.cpp
)

add_library(enttecwing SHARED ${enttecwing_src})

target_link_libraries(enttecwing Qt5::Widgets Qt5::Network)

add_executable(enttecwing_test 
    enttecwing/test/main.cpp
    enttecwing/test/playbackwing_test.cpp
    enttecwing/test/programwing_test.cpp
    enttecwing/test/shortcutwing_test.cpp
    enttecwing/test/wing_test.cpp
)

target_link_libraries(enttecwing_test enttecwing Qt5::Test Qt5::Core)

add_test(enttecwing_test enttecwing_test)

install(TARGETS enttecwing DESTINATION ${PLUGINDIR})
install(FILES enttecwing/src/org.qlcplus.QLCPlus.enttecwing.metainfo.xml DESTINATION ${METAINFODIR})

#gpio plugin

set(gpio_src
    interfaces/qlcioplugin.cpp
    gpio/gpioconfiguration.cpp
    gpio/gpioplugin.cpp
    gpio/gpioreaderthread.cpp
)

add_library(gpio SHARED ${gpio_src})
install(TARGETS gpio DESTINATION ${PLUGINDIR})

#hid plugin

set(hid_src
    interfaces/qlcioplugin.cpp
    hid/configurehid.cpp
    hid/hiddevice.cpp
    hid/hiddmxdevice.cpp
    hid/hidjsdevice.cpp
    hid/hidplugin.cpp
)

IF (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    LIST(APPEND hid_src
        hid/win32/hidapi.cpp
        hid/win32/hidwindowsjoystick.cpp
    )
    set(hidlink Qt5::Widgets setupapi winmm)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    LIST(APPEND hid_src
        hid/macx/hidapi.cpp
        hid/macx/hidosxjoystick.cpp
    )
    set(hidlink Qt5::Widgets IOKit CoreFoundation)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    LIST(APPEND hid_src
        hid/linux/hidapi.cpp
        hid/linux/hidlinuxjoystick.cpp
    )
    install(FILES hid/linux/z65-fx5-hid.rules DESTINATION ${UDEVRULESDIR})
    install(FILES hid/linux/org.qlcplus.QLCPlus.hid.metainfo.xml DESTINATION ${METAINFODIR})
ENDIF()

add_library(hid SHARED ${hid_src})
target_link_libraries(hid Qt5::Widgets ${hidlink})
install(TARGETS hid DESTINATION ${PLUGINDIR})

#loopback plugin
add_library(loopback SHARED 
    interfaces/qlcioplugin.cpp 
    loopback/src/loopback.cpp
)

target_link_libraries(loopback Qt5::Core)
install(TARGETS loopback DESTINATION ${PLUGINDIR})
install(FILES loopback/src/org.qlcplus.QLCPlus.loopback.metainfo.xml DESTINATION ${METAINFODIR})

#midi plugin

set(midiplugin_src
    interfaces/qlcioplugin.cpp
    midi/src/common/configuremidiplugin.cpp
    midi/src/common/mididevice.cpp
    midi/src/common/midiinputdevice.cpp
    midi/src/common/midioutputdevice.cpp
    midi/src/common/midiplugin.cpp
    midi/src/common/midiprotocol.cpp
    midi/src/common/miditemplate.cpp
)

IF (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    LIST(APPEND midiplugin_src
        midi/src/win32/win32midienumerator.cpp
        midi/src/win32/win32midiinputdevice.cpp
        midi/src/win32/win32midioutputdevice.cpp
    )
    set(midilink setupapi winmm)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    LIST(APPEND midiplugin_src
        midi/src/macx/coremidienumerator.cpp
        midi/src/macx/coremidiinputdevice.cpp
        midi/src/macx/coremidioutputdevice.cpp
    )
    set(midilink CoreMIDI CoreFoundation)
ELSE()
    IF(ALSA_FOUND)
        list(APPEND INCLUDE_DIRECTORIES
            ${ALSA_INCLUDE_DIRS}
        )
        LIST(APPEND midiplugin_src
            midi/src/alsa/alsamidienumerator.cpp
            midi/src/alsa/alsamidiinputdevice.cpp
            midi/src/alsa/alsamidiinputthread.cpp
            midi/src/alsa/alsamidioutputdevice.cpp
            midi/src/alsa/alsamidiutil.cpp
        )
        set(midilink ${ALSA_LIBRARIES})
    ENDIF()
ENDIF()

add_library(midiplugin SHARED 
    ${midiplugin_src}
)

target_link_libraries(midiplugin Qt5::Widgets ${midilink} )
install(TARGETS midiplugin DESTINATION ${PLUGINDIR})
install(FILES midi/src/org.qlcplus.QLCPlus.midi.metainfo.xml DESTINATION ${METAINFODIR})
