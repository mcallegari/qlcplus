include(../../../variables.cmake)
include(../../../coverage.cmake)

add_executable(inputoutputmap_test WIN32
    ../common/resource_paths.h
    inputoutputmap_test.cpp inputoutputmap_test.h
)
target_include_directories(inputoutputmap_test PRIVATE
    ../../../plugins/interfaces
    ../../src
    ../iopluginstub
)

target_link_libraries(inputoutputmap_test PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Test
    qlcplusengine
)

if(IS_TRAVIS___contains___ON)
    target_compile_definitions(inputoutputmap_test PRIVATE
        SKIP_TEST
    )
endif()

# if(count(IS_BUILDBOT, 1))
if(IS_BUILDBOT)
    target_compile_definitions(inputoutputmap_test PRIVATE
        SKIP_TEST
    )
endif()

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
