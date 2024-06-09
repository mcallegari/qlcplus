cmake_minimum_required(VERSION 3.16)
project(icons VERSION 1.0 LANGUAGES C CXX)

#set(CMAKE_INCLUDE_CURRENT_DIR ON)
include(../../variables.cmake)

# install macOS files for a correct bundle
install(FILES qt.conf DESTINATION ${INSTALLROOT}/${DATADIR})
install(FILES ../../resources/icons/qlcplus.icns DESTINATION ${INSTALLROOT}/${DATADIR})

if(qmlui)
    install(FILES Info.plist.qmlui DESTINATION ${INSTALLROOT}/Info.plist)
else()
    install(FILES Info.plist DESTINATION ${INSTALLROOT})
endif()
install(CODE "execute_process(COMMAND sed -i -e \"s/__QLC_VERSION__/${APPVERSION}/g\" ${INSTALLROOT}/Info.plist)")

# install Qt library frameworks
set(QT_FRAMEWORKS_DIR ${_qt5_root_dir}/../../Frameworks)
set(QT_FRAMEWORK_NAMES
    "QtCore"
    "QtDBus"
    "QtGui"
    "QtMultimedia"
    "QtMultimediaWidgets"
    "QtNetwork"
    "QtOpenGL"
    "QtPrintSupport"
    "QtScript"
    "QtSerialPort"
    "QtSvg"
    "QtWidgets"
)
# Loop through each framework and copy it to installation
foreach(FW_NAME IN LISTS QT_FRAMEWORK_NAMES)
    # install each framework Version file
    install(FILES ${QT_FRAMEWORKS_DIR}/${FW_NAME}.framework/Versions/5/${FW_NAME}
            DESTINATION ${INSTALLROOT}/${LIBSDIR}/${FW_NAME}.framework/Versions/5)
endforeach()

# install Qt plugins
set(QT_PLUGINS_DIR ${_qt5_root_dir}/../../plugins)
# imageformats
install(FILES ${QT_PLUGINS_DIR}/imageformats/libqgif.dylib DESTINATION ${INSTALLROOT}/${PLUGINDIR}/imageformats)
install(FILES ${QT_PLUGINS_DIR}/imageformats/libqjpeg.dylib DESTINATION ${INSTALLROOT}/${PLUGINDIR}/imageformats)
install(FILES ${QT_PLUGINS_DIR}/imageformats/libqsvg.dylib DESTINATION ${INSTALLROOT}/${PLUGINDIR}/imageformats)
# multimedia
install(FILES ${QT_PLUGINS_DIR}/mediaservice/libqavfmediaplayer.dylib DESTINATION ${INSTALLROOT}/${PLUGINDIR}/mediaservice)
install(FILES ${QT_PLUGINS_DIR}/mediaservice/libqtmedia_audioengine.dylib DESTINATION ${INSTALLROOT}/${PLUGINDIR}/mediaservice)
# platforms
install(FILES ${QT_PLUGINS_DIR}/platforms/libqcocoa.dylib DESTINATION ${INSTALLROOT}/${PLUGINDIR}/platforms)
# styles
install(FILES ${QT_PLUGINS_DIR}/styles/libqmacstyle.dylib DESTINATION ${INSTALLROOT}/${PLUGINDIR}/styles)

# install support libraries
pkg_check_modules(SNDFILE IMPORTED_TARGET flac)
pkg_check_modules(SNDFILE IMPORTED_TARGET ogg)
pkg_check_modules(SNDFILE IMPORTED_TARGET opus)
pkg_check_modules(SNDFILE IMPORTED_TARGET vorbis)
pkg_check_modules(SNDFILE IMPORTED_TARGET vorbisenc)

install(FILES "${pkgcfg_lib_FFTW3_fftw3}" DESTINATION ${INSTALLROOT}/${LIBSDIR})
install(FILES "${pkgcfg_lib_LIBFTDI1_ftdi1}" DESTINATION ${INSTALLROOT}/${LIBSDIR})
install(FILES "${pkgcfg_lib_LIBUSB1_usb-1.0}" DESTINATION ${INSTALLROOT}/${LIBSDIR})
install(FILES "${pkgcfg_lib_MAD_mad}" DESTINATION ${INSTALLROOT}/${LIBSDIR})
install(FILES "${pkgcfg_lib_SNDFILE_sndfile}" DESTINATION ${INSTALLROOT}/${LIBSDIR})
install(FILES "${pkgcfg_lib_SNDFILE_FLAC}" DESTINATION ${INSTALLROOT}/${LIBSDIR})
install(FILES "${pkgcfg_lib_SNDFILE_ogg}" DESTINATION ${INSTALLROOT}/${LIBSDIR})
install(FILES "${pkgcfg_lib_SNDFILE_opus}" DESTINATION ${INSTALLROOT}/${LIBSDIR})
install(FILES "${pkgcfg_lib_SNDFILE_vorbis}" DESTINATION ${INSTALLROOT}/${LIBSDIR})
install(FILES "${pkgcfg_lib_SNDFILE_vorbisenc}" DESTINATION ${INSTALLROOT}/${LIBSDIR})

# HELPER: PRINT ALL VARIABLES KNOWN BY CMAKE

#message("QTDIR: ${QT_DIR}")
#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
