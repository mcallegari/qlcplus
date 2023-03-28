include(../../../variables.cmake)
include(../../../coverage.cmake)

add_library(loopback)
target_sources(loopback PRIVATE
    ../../interfaces/qlcioplugin.cpp ../../interfaces/qlcioplugin.h
    loopback.cpp loopback.h
)
target_include_directories(loopback PRIVATE
    ../../interfaces
)

target_link_libraries(loopback PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
)

if(WIN32)
    target_compile_definitions(loopback PRIVATE
        QLC_EXPORT
    )
endif()

install(TARGETS loopback
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
