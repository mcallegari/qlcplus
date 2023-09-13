project(gobo)

set(GOBO_FILES Chauvet ClayPaky GLP Others Robe SGM SGM-Color)

install(DIRECTORY ${GOBO_FILES} DESTINATION ${INSTALLROOT}/${GOBODIR})
