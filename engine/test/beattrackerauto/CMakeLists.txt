add_executable(beattrackerauto_test WIN32
    beattrackerauto_test.cpp beattrackerauto_test.h
)
target_include_directories(beattrackerauto_test PRIVATE
    ../../../plugins/interfaces
    ../../audio/src
    ../../src
)

target_link_libraries(beattrackerauto_test PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Test
    qlcplusengine
)
