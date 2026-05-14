add_executable(webaccessescaping_test WIN32 MACOSX_BUNDLE
    ../src/commonjscss.cpp
    webaccessescaping_test.cpp webaccessescaping_test.h
)
target_include_directories(webaccessescaping_test PRIVATE
    ../src
)
target_link_libraries(webaccessescaping_test PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Test
)
