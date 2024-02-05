set(module_name "mastertimer_test")

add_executable(${module_name} WIN32
    # ../../../plugins/interfaces/qlcioplugin.cpp
    ../common/resource_paths.h
    ../function/function_stub.cpp ../function/function_stub.h
    dmxsource_stub.cpp dmxsource_stub.h
    ${module_name}.cpp ${module_name}.h
)
target_include_directories(${module_name} PRIVATE
    ../../../plugins/interfaces
    ../../src
    ../function
)

target_link_libraries(${module_name} PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Test
    qlcplusengine
)

if ("$ENV{TRAVIS}" STREQUAL "true")
    target_compile_definitions(${module_name} PRIVATE
        SKIP_TEST
    )
endif()

#if(WIN32)
#    set(CURRUSR $ENV{USERNAME})
#else()
#    set(CURRUSR $ENV{USER})
#endif()

#string(STRIP ${CURRUSR} CURRUSR)
#string(FIND ${CURRUSR} "build" IS_BUILDBOT)
#if(IS_BUILDBOT GREATER -1)
#    target_compile_definitions(${module_name} PRIVATE
#        SKIP_TEST
#    )
#endif()

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
