
add_library(iopluginstub SHARED)
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
