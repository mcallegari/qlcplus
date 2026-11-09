set(module_name "vclabel_test")

add_executable(${module_name} WIN32
    ${module_name}.cpp ${module_name}.h
)

target_link_libraries(${module_name} PRIVATE
    qmluitestlib
)
