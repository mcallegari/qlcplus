project(engine VERSION 1.0.0)

pkg_check_modules(FFTW3 IMPORTED_TARGET fftw3)

add_subdirectory(audio)
add_subdirectory(src)
if(NOT ANDROID AND NOT IOS)
    add_subdirectory(test)
endif()
