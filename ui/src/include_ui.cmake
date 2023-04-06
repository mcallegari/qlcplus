get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

# Function Name: include_ui_header
# This function is called to add auto-generated ui header files to include directory of the module
function(include_ui_header module_name)
    if(_isMultiConfig)
        target_include_directories(${module_name} PRIVATE
            ${CMAKE_BINARY_DIR}/ui/src/qlcplusui_autogen/include_${CONFIG}
        )
    else()
        target_include_directories(${module_name} PRIVATE
            ${CMAKE_BINARY_DIR}/ui/src/qlcplusui_autogen/include
        )
    endif()
endfunction()

