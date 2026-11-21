project(windows)

# Set paths
set(SYS_LIBS_PATH $ENV{SystemDrive}/msys64/clang64/bin)

function(copy_system_library target libname)
    if(EXISTS ${SYS_LIBS_PATH}/${libname})
        list(APPEND ${target} ${SYS_LIBS_PATH}/${libname})
        set(${target} "${${target}}" PARENT_SCOPE)
    else()
        message(FATAL_ERROR "ERROR: missing DLL detected: ${libname}")
    endif()
endfunction()

# MSYS2 libraries
set(msys_path "${INSTALLROOT}/${LIBSDIR}")
copy_system_library(msys_files "libusb-1.0.dll")

# C++ runtime. These must come from the MSYS2 CLANG64 toolchain we build with,
# NOT from Qt: the libc++.dll shipped with Qt's LLVM-MinGW kit does not export
# the unwind ABI (_Unwind_Resume and friends), which our binaries import from
# libc++.dll. Deploying Qt's copy makes every executable fail to start with
# "The procedure entry point _Unwind_Resume could not be located".
# Pass --no-compiler-runtime to windeployqt so it does not overwrite these.
copy_system_library(msys_files "libc++.dll")
copy_system_library(msys_files "libunwind.dll")
install(FILES ${msys_files} DESTINATION ${msys_path})

# audio libraries
set(mmedia_path "${INSTALLROOT}/${LIBSDIR}")
copy_system_library(mmedia_files "libogg-0.dll")
copy_system_library(mmedia_files "libopus-0.dll")
copy_system_library(mmedia_files "libmp3lame-0.dll")
copy_system_library(mmedia_files "libmpg123-0.dll")
copy_system_library(mmedia_files "libvorbis-0.dll")
copy_system_library(mmedia_files "libvorbisenc-2.dll")
copy_system_library(mmedia_files "libFLAC.dll")
copy_system_library(mmedia_files "libsndfile-1.dll")

install(FILES ${mmedia_files} DESTINATION ${mmedia_path})

# NullSoft installer files
set(nsis_path "${INSTALLROOT}/${DATADIR}")
if(qmlui)
    set(nsis_files qlcplus5Qt6.nsi)
else()
    set(nsis_files qlcplus4Qt6.nsi)
endif()
install(FILES ${nsis_files} DESTINATION ${nsis_path})

# install(FILES ../Sample.qxw DESTINATION ${INSTALLROOT}/${DATADIR})
