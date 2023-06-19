project(plugins)

if((((NOT ANDROID AND NOT IOS))))
    pkg_check_modules(MAD IMPORTED_TARGET mad)
    if(${MAD_FOUND})
        add_subdirectory(mad)
    endif()

    pkg_check_modules(SNDFILE IMPORTED_TARGET sndfile)
    if(${SNDFILE_FOUND})
        add_subdirectory(sndfile)
    endif()
endif()
