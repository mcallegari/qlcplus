include_directories(
    interfaces
    artnet/src/
    dmxusb/src/
    midi/src/common
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Test_INCLUDE_DIRS}
    ${Qt5SerialPort_INCLUDE_DIRS}
    ${FTDI_INCLUDE_DIRS} OPTIONAL
    ${LIBUSB_1_INCLUDE_DIRS} OPTIONAL
)

#arnet plugin
set(artnet_src
    interfaces/qlcioplugin.cpp
    artnet/src/artnetcontroller.cpp
    artnet/src/artnetpacketizer.cpp
    artnet/src/artnetplugin.cpp
    artnet/src/configureartnet.cpp
)

add_library(artnet SHARED ${artnet_src})

target_link_libraries(artnet Qt5::Widgets Qt5::Network)

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
#     dmxusb/src/ftd2xx-interface.cpp
    dmxusb/src/stageprofi.cpp
    dmxusb/src/vinceusbdmx512.cpp
)

if(${FTDI_FOUND})
  message(STATUS "Building with libFTDI1 support")
  add_definitions(-DLIBFTDI1)
  LIST(APPEND dmxusb_src 
    dmxusb/src/libftdi-interface.cpp
    dmxusb/src/nanodmx.cpp
    dmxusb/src/euroliteusbdmxpro.cpp
  )
  IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    install(FILES dmxusb/src/z65-dmxusb.rules DESTINATION ${UDEVRULESDIR})
  ENDIF()
else()
  message(STATUS "Building with QtSerial support")
  add_definitions(-DQTSERIAL)
  LIST(APPEND dmxusb_src dmxusb/src/qtserial-interface.cpp)
endif()

add_library(dmxusb SHARED ${dmxusb_src})

target_link_libraries(dmxusb
    Qt5::Core
    Qt5::Widgets
    Qt5::SerialPort
    ${FTDI_LIBRARIES}
    ${LIBUSB_1_LIBRARIES}
)

install(TARGETS dmxusb DESTINATION ${PLUGINDIR})
install(FILES dmxusb/src/org.qlcplus.QLCPlus.dmxusb.metainfo.xml DESTINATION ${METAINFODIR})
