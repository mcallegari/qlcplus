project(idn)

message(STATUS "Building IDN plugin")

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

set(module_name "idn")

add_library(${module_name}
    SHARED
)

target_sources(${module_name} PRIVATE
    ../interfaces/qlcioplugin.cpp
    ../interfaces/qlcioplugin.h

    idn.cpp
    idn.h
    idnclient.cpp
    idnclient.h
    idnconfiguration.cpp
    idnconfiguration.h
    idnconfiguration.ui
    idncontroller.cpp
    idncontroller.h
    idnoptimizer.cpp
    idnoptimizer.h
    idnpacketizer.cpp
    idnpacketizer.h
    ui_idnconfiguration.h
    nlohmann/json.hpp
)

target_include_directories(${module_name} PRIVATE
    ../interfaces
)

target_link_libraries(${module_name} PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Network
    Qt${QT_MAJOR_VERSION}::Widgets
)

install(TARGETS ${module_name}
    LIBRARY DESTINATION ${INSTALLROOT}/${PLUGINDIR}
    RUNTIME DESTINATION ${INSTALLROOT}/${PLUGINDIR}
)