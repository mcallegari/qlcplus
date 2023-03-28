include(../../../variables.cmake)
include(../../../coverage.cmake)

add_library(enttecwing)
target_sources(enttecwing PRIVATE
    ../../interfaces/qlcioplugin.cpp ../../interfaces/qlcioplugin.h
    enttecwing.cpp enttecwing.h
    playbackwing.cpp playbackwing.h
    programwing.cpp programwing.h
    shortcutwing.cpp shortcutwing.h
    wing.cpp wing.h
)
target_include_directories(enttecwing PRIVATE
    ../../interfaces
)

target_link_libraries(enttecwing PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Network
    Qt${QT_MAJOR_VERSION}::Widgets
)

if(WIN32)
    target_compile_definitions(enttecwing PRIVATE
        QLC_EXPORT
    )
endif()

install(TARGETS enttecwing
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
