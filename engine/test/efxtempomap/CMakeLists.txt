add_executable(efxtempomap_test WIN32
    ../common/resource_paths.h
    efxtempomap_test.cpp efxtempomap_test.h
)
target_include_directories(efxtempomap_test PRIVATE
    ../../../plugins/interfaces
    ../../src
)

target_link_libraries(efxtempomap_test PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Test
    qlcplusengine
)

if(APPLE)
    set_target_properties(efxtempomap_test PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()
