CONFIG(coverage) {
    # Code coverage measurement with gcov
    QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
    LIBS += -lgcov
    QMAKE_CLEAN += *.gcno *.gcov *.gcda
    QMAKE_DISTCLEAN += *.gcno *.gcov *.gcda
}
