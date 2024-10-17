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
#set(QT_FRAMEWORKS_DIR ${_qt5_root_dir}/../../Frameworks) # homebrew
set(QT_FRAMEWORKS_DIR ${_qt5_root_dir}/..)
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
    "QtWebSockets"
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
install(FILES ${FFTW3_LIBDIR}/libfftw3.3.dylib DESTINATION ${INSTALLROOT}/${LIBSDIR})
install(FILES ${LIBFTDI1_libftdi1_LIBDIR}/libftdi1.2.dylib DESTINATION ${INSTALLROOT}/${LIBSDIR})
install(FILES ${LIBFTDI1_libftdi1_LIBDIR}/libftdi1.2.5.0.dylib DESTINATION ${INSTALLROOT}/${LIBSDIR})
install(FILES ${LIBUSB1_LIBDIR}/libusb-1.0.0.dylib DESTINATION ${INSTALLROOT}/${LIBSDIR})
install(FILES ${MAD_LIBDIR}/libmad.0.dylib DESTINATION ${INSTALLROOT}/${LIBSDIR})
install(FILES ${SNDFILE_LIBDIR}/libsndfile.1.dylib DESTINATION ${INSTALLROOT}/${LIBSDIR})
install(FILES ${SNDFILE_LIBDIR}/libsndfile.1.0.37.dylib DESTINATION ${INSTALLROOT}/${LIBSDIR})

#pkg_check_modules(FLAC IMPORTED_TARGET flac)
#pkg_check_modules(OGG IMPORTED_TARGET ogg)
#pkg_check_modules(OPUS IMPORTED_TARGET opus)
#pkg_check_modules(MPG123 IMPORTED_TARGET libmpg123)
#pkg_check_modules(LAME IMPORTED_TARGET lame)
#pkg_check_modules(VORBIS IMPORTED_TARGET vorbis)
#pkg_check_modules(VORBISENC IMPORTED_TARGET vorbisenc)
#install(FILES ${FLAC_LIBDIR}/libFLAC.12.dylib DESTINATION ${INSTALLROOT}/${LIBSDIR})
#install(FILES ${OGG_LIBDIR}/libogg.0.dylib DESTINATION ${INSTALLROOT}/${LIBSDIR})
#install(FILES ${OGG_LIBDIR}/libogg.0.8.5.dylib DESTINATION ${INSTALLROOT}/${LIBSDIR})
#install(FILES ${OPUS_LIBDIR}/libopus.0.dylib DESTINATION ${INSTALLROOT}/${LIBSDIR})
#install(FILES ${MPG123_LIBDIR}/libmpg123.0.dylib DESTINATION ${INSTALLROOT}/${LIBSDIR})
#install(FILES ${LAME_LIBDIR}/libmp3lame.0.dylib DESTINATION ${INSTALLROOT}/${LIBSDIR})
#install(FILES ${VORBIS_LIBDIR}/libvorbis.0.dylib DESTINATION ${INSTALLROOT}/${LIBSDIR})
#install(FILES ${VORBISENC_LIBDIR}/libvorbisenc.2.dylib DESTINATION ${INSTALLROOT}/${LIBSDIR})

#install(FILES ${FFTW3_LIBDIR}/libpcre2-16.0.dylib DESTINATION ${INSTALLROOT}/${LIBSDIR})
# more deps: libzstd.1.dylib #libgthread-2.0.0.dylib #libglib-2.0.0.dylib #libintl.8.dylib
#            libpng16.16.dylib #libqjpeg.dylib #libjpeg.8.dylib #libmp3lame.0.dylib

# HELPER: PRINT ALL VARIABLES KNOWN BY CMAKE

#message("QTDIR: ${QT_DIR}")
#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
