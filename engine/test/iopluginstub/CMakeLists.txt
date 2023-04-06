include(../../../variables.cmake)

add_library(iopluginstub)
target_sources(iopluginstub PRIVATE
    ../../../plugins/interfaces/qlcioplugin.cpp ../../../plugins/interfaces/qlcioplugin.h
    iopluginstub.cpp iopluginstub.h
)
target_include_directories(iopluginstub PRIVATE
    ../../../plugins/interfaces
)

target_link_libraries(iopluginstub PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
)

install(TARGETS iopluginstub
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
