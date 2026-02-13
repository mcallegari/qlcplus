add_executable(oscpacketizer_test WIN32
    oscpacketizer_test.cpp oscpacketizer_test.h
    ../oscpacketizer.cpp ../oscpacketizer.h
)
target_include_directories(oscpacketizer_test PRIVATE
    ..
)

target_link_libraries(oscpacketizer_test PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Network
    Qt${QT_MAJOR_VERSION}::Test
)
