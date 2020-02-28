set(artnet_src
    interfaces/qlcioplugin.cpp
    artnet/src/artnetcontroller.cpp
    artnet/src/artnetpacketizer.cpp
    artnet/src/artnetplugin.cpp
    artnet/src/configureartnet.cpp
)

include_directories(
    interfaces
    artnet/src/
    ${Qt5Network_INCLUDE_DIRS}
    ${Qt5Test_INCLUDE_DIRS}
)

add_library(artnet SHARED ${artnet_src})

target_link_libraries(artnet Qt5::Widgets Qt5::Network)

add_executable(artnet_test artnet/test/artnet_test.cpp)

target_link_libraries(artnet_test artnet Qt5::Test)

install(TARGETS artnet DESTINATION ${PLUGINDIR})
install(FILES artnet/src/org.qlcplus.QLCPlus.artnet.metainfo.xml DESTINATION ${METAINFODIR})
