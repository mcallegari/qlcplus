cmake_minimum_required (VERSION 3.0.0)
project (qlcplus VERSION 4.13.0)

enable_testing(true)

IF (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(APPROOT "ENV{SystemDrive}/qlcplus")
    set(LIBSDIR "${APPROOT}")
    set(DATADIR ".")
    set(USERDATADIR "QLC+")
    set(DOCSDIR "Documents")
    set(INPUTPROFILEDIR "InputProfiles")
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(APPROOT "~/QLC+.app/Contents")
    set(LIBSDIR "~/Library")
    set(DATADIR "resources")
    set(USERDATADIR "Library/Application Support/QLC+")
    set(DOCSDIR "${DATADIR}/Documents")
    set(INPUTPROFILEDIR "${DATADIR}/InputProfiles")
    set(CMAKE_MACOSX_RPATH 1)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set(APPROOT "/")
    set(LIBSDIR "${CMAKE_INSTALL_PREFIX}/lib/qlcplus")
    set(DATADIR "assets")
    set(USERDATADIR ".qlcplus")
    set(DOCSDIR "${DATADIR}/Documents")
    set(INPUTPROFILEDIR "${DATADIR}/inputprofiles")
ELSE()
    set(APPROOT ${CMAKE_INSTALL_PREFIX})
    set(LIBSDIR "${CMAKE_INSTALL_PREFIX}/lib/qlcplus")
    set(DATADIR "${CMAKE_INSTALL_PREFIX}/share/qlcplus")
    set(USERDATADIR ".qlcplus")
    set(DOCSDIR "${DATADIR}/Documents")
    set(INPUTPROFILEDIR "${DATADIR}/inputprofiles")
ENDIF()

set(APPNAME        "Q Light Controller Plus")
set(FXEDNAME       "Fixture Definition Editor")
set(TRANSLATIONDIR "${APPROOT}/share/qlcplus/translations")
set(PLUGINDIR      "${APPROOT}/lib/qlcplus/plugins")
set(AUDIOPLUGINDIR "${PLUGINDIR}/audio")
set(RGBSCRIPTDIR   "${DATADIR}/rgbscripts")
set(USERRGBSCRIPTDIR "${USERDATADIR}/rgbscripts")
set(USERINPUTPROFILEDIR "${USERDATADIR}/inputprofiles")
set(WEBFILESDIR     "web")
set(GOBODIR         "gobos")
set(FIXTUREDIR      "${DATADIR}/fixtures")
set(USERFIXTUREDIR  "${USERDATADIR}/fixtures")
set(METAINFODIR     "${APPROOT}/share/metainfo")
set(UDEVRULESDIR    "/etc/udev/rules.d")

set(CMAKE_INSTALL_RPATH ${LIBSDIR})

set(CMAKE_MODULE_PATH 
    ${CMAKE_MODULE_PATH}
    ${PROJECT_SOURCE_DIR}/cmake
)

find_package(Qt5 COMPONENTS 
    Core
    Widgets 
    Script 
    Multimedia 
    Qml 
    Xml 
    Gui
    LinguistTools
    Network
    MultimediaWidgets REQUIRED
    SerialPort
    Test
)

find_package(FTDI)

find_package(libusb-1.0)

find_package(FTD2XX)

find_package(ALSA)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

IF( ${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
ELSE()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpic")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fpic")
ENDIF()

list(APPEND languages 
    de_DE
    es_ES
    fr_FR
    it_IT
    nl_NL 
    cz_CZ
    pt_BR
    ca_ES
    ja_JP
)

foreach(lang LISTS ${languages})
    file(GLOB_RECURSE tsfiles LIST_DIRECTORIES FALSE RELATIVE ${CMAKE_CURRENT_BINARY_DIR} "*_${lang}.ts")
    list(FILTER tsfiles EXCLUDE REGEX "/qmlui/*")
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/qlcplus_${lang}"
        COMMAND ${Qt5_LUPDATE_EXECUTABLE}
        ARGS ${tsfiles}  -qm "qlcplus_${lang}"
        VERBATIM
    )
endforeach()

#$LRELEASE_BIN -silent $FILELIST -qm $OUTPUT_NAME

file(GLOB_RECURSE qmfiles "${CMAKE_CURRENT_BINARY_DIR}/*.qm" LIST_DIRECTORIES false)

install(FILES ${qmfiles} DESTINATION ${TRANSLATIONDIR})

install(DIRECTORY resources/ DESTINATION ${DATADIR} 
        PATTERN "Makefile" EXCLUDE
        PATTERN "*.pro" EXCLUDE
)

add_subdirectory(hotplugmonitor)
add_subdirectory(engine)
add_subdirectory(ui)
add_subdirectory(webaccess)
add_subdirectory(launcher)
add_subdirectory(main)
add_subdirectory(plugins)
