set(module_name "link")

# The Ableton Link SDK is a git submodule. Build this plugin only when it has
# been checked out with: git submodule update --init --recursive
set(LINK_SDK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ableton-link)

if(NOT EXISTS ${LINK_SDK_DIR}/include/ableton/Link.hpp OR
   NOT EXISTS ${LINK_SDK_DIR}/modules/asio-standalone/include/asio/version.hpp)
    message(STATUS "Ableton Link SDK not found: the Link service plugin will not be built")
    return()
endif()

include(${LINK_SDK_DIR}/AbletonLinkConfig.cmake)

add_library(${module_name} ${QLC_PLUGIN_TYPE}
    ../../interfaces/qlcserviceplugin.h
    ../../interfaces/qlclinkservice.h
    linkplugin.cpp linkplugin.h linkplugin.json
    linkservice.cpp linkservice.h
)

target_include_directories(${module_name} PRIVATE
    ../../interfaces
)

target_compile_features(${module_name} PRIVATE cxx_std_17)

target_link_libraries(${module_name} PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Ableton::Link
)

if(WIN32)
    # Link must see the same _WIN32_WINNT everywhere in the target
    link_pin_win32_winnt(${module_name})
    target_compile_definitions(${module_name} PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
    # MinGW ignores the #pragma comment(lib) used by the Link headers
    target_link_libraries(${module_name} PRIVATE ws2_32 mswsock iphlpapi avrt)
elseif(NOT MSVC)
    # The Link SDK uses multi-character constants
    target_compile_options(${module_name} PRIVATE -Wno-multichar)
endif()

install(TARGETS ${module_name}
    LIBRARY DESTINATION ${INSTALLROOT}/${PLUGINDIR}/services
    RUNTIME DESTINATION ${INSTALLROOT}/${PLUGINDIR}/services
)
