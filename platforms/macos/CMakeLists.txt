cmake_minimum_required(VERSION 3.16)
project(icons VERSION 1.0 LANGUAGES C CXX)

#set(CMAKE_INCLUDE_CURRENT_DIR ON)
include(../../variables.cmake)

# install macOS files for a correct bundle
install(FILES qt.conf DESTINATION ${INSTALLROOT}/${DATADIR})
install(FILES ../../resources/icons/qlcplus.icns DESTINATION ${INSTALLROOT})

if(qmlui)
    install(FILES Info.plist.qmlui DESTINATION ${INSTALLROOT}/Info.plist)
else()
    install(FILES Info.plist DESTINATION ${INSTALLROOT})
endif()
install(CODE "execute_process(COMMAND sed -i -e \"s/__QLC_VERSION__/${APPVERSION}/g\" ${INSTALLROOT}/Info.plist)")
