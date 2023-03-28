include(../../variables.cmake)

qt_add_plugin(olaio)
target_sources(olaio PRIVATE
    ../interfaces/qlcioplugin.cpp ../interfaces/qlcioplugin.h
    configureolaio.cpp configureolaio.h configureolaio.ui
    olaio.cpp olaio.h
    olaoutthread.cpp olaoutthread.h
    qlclogdestination.cpp qlclogdestination.h
)
target_include_directories(olaio PRIVATE
    ../interfaces
)

target_link_libraries(olaio PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
)

if(MACOS)
    set_target_properties(olaio PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

if((MACOS) AND (NOT (packagesExist(libolalibolaserver))))
    target_include_directories(olaio PRIVATE
        /opt/local/include
    )

    target_link_libraries(olaio PRIVATE
        # Remove: L/opt/local/lib
        ola
        olacommon
        olaserver
    )
endif()

if(NOT MACOS)
    target_link_libraries(olaio PRIVATE
        # Remove: L/usr/local/lib
        ola
        olacommon
        olaserver
    )
endif()

install(TARGETS olaio
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
