cmake_minimum_required (VERSION 3.0.0)
project (qlcplus VERSION 4.13.0)

enable_testing(true)

IF (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(APPROOT "ENV{SystemDrive}/qlcplus")
    set(DATADIR "")
    set(USERDATADIR "QLC+")
    set(DOCSDIR "Documents")
    set(INPUTPROFILEDIR "InputProfiles")
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(APPROOT "~/QLC+.app/Contents")
    set(DATADIR "Resources")
    set(USERDATADIR "Library/Application Support/QLC+")
    set(DOCSDIR "${DATADIR}/Documents")
    set(INPUTPROFILEDIR "${DATADIR}/InputProfiles")
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set(APPROOT "/")
    set(DATADIR "assets")
    set(USERDATADIR ".qlcplus")
    set(DOCSDIR "${DATADIR}/Documents")
    set(INPUTPROFILEDIR "${DATADIR}/inputprofiles")
ELSE()
    set(APPROOT ${CMAKE_INSTALL_PREFIX})
    set(DATADIR "share/qlcplus")
    set(USERDATADIR ".qlcplus")
    set(DOCSDIR "${DATADIR}/Documents")
    set(INPUTPROFILEDIR "${DATADIR}/inputprofiles")
ENDIF()

set(APPNAME        "Q Light Controller Plus")
set(FXEDNAME       "Fixture Definition Editor")
set(TRANSLATIONDIR "${APPROOT}/share/qlcplus/translations")
set(AUDIOPLUGINDIR "${APPROOT}/lib/qlcplus/PlugIns/Audio")
set(PLUGINDIR      "${APPROOT}/lib/qlcplus/Plugins")
set(RGBSCRIPTDIR   "${DATADIR}/RGBScripts")
set(LIBSDIR        "${CMAKE_LIBRARY_PATH}")
set(USERRGBSCRIPTDIR "${USERDATADIR}/RGBScripts")
set(USERINPUTPROFILEDIR "${USERDATADIR}/InputProfiles")
set(WEBFILESDIR     "${DATADIR}/web")

find_package(Qt5 COMPONENTS 
    Core 
    Widgets 
    Script 
    Multimedia 
    Qml 
    Xml 
    Gui
    LinguistTools
    MultimediaWidgets REQUIRED
)

find_package(Qt5Test REQUIRED)

set(CMAKE_MODULE_PATH 
    ${CMAKE_MODULE_PATH}
    ${PROJECT_SOURCE_DIR}/cmake
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpic")

add_subdirectory(hotplugmonitor)
add_subdirectory(engine)
add_subdirectory(ui)
add_subdirectory(webaccess)
add_subdirectory(launcher)
add_subdirectory(main)

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

install(DIRECTORY resources DESTINATION ${DATADIR})
