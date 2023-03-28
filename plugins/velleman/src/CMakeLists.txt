include(../../../variables.cmake)
include(../../../coverage.cmake)

add_library(velleman)
target_sources(velleman PRIVATE
    ../../interfaces/qlcioplugin.cpp ../../interfaces/qlcioplugin.h
    velleman.cpp velleman.h
)
target_include_directories(velleman PRIVATE
    ../../interfaces
)

target_link_libraries(velleman PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
)

if(WIN32)
    target_compile_definitions(velleman PRIVATE
        QLC_EXPORT
    )

    target_link_libraries(velleman PRIVATE
        # Remove: LC:/Qt/K8062D
        C:/Qt/K8062D/K8062D.a
        K8062D
    )
endif()

if(UNIX)
    target_sources(velleman PRIVATE
        velleman_mock.cpp
    )
endif()

install(TARGETS velleman
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
