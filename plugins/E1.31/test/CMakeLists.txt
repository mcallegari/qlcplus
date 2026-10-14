add_executable(e131packetizer_test WIN32 MACOSX_BUNDLE
    ../e131packetizer.cpp ../e131packetizer.h
    e131packetizer_test.cpp e131packetizer_test.h
)

target_include_directories(e131packetizer_test PRIVATE
    ..
)

target_link_libraries(e131packetizer_test PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Network
    Qt${QT_MAJOR_VERSION}::Test
)
