LIBFFTW_DIR      = $$system("pkg-config --variable libdir fftw3")
LIBFFTW_FILE     = libfftw3.3.dylib
LIBFFTW_FILEPATH = $$LIBFFTW_DIR/$$LIBFFTW_FILE

LIBFFTW_INSTALL_NAME_TOOL = install_name_tool -change $$LIBFFTW_FILEPATH \
                @executable_path/../$$LIBSDIR/$$LIBFFTW_FILE

contains(PKGCONFIG, mad) {
    !isEmpty(nametool.commands) {
        nametool.commands += "&&" 
    }

    nametool.commands += $$LIBFFTW_INSTALL_NAME_TOOL $$OUTFILE
}

LIBFFTW.path   = $$INSTALLROOT/$$LIBSDIR
LIBFFTW.files += $$LIBFFTW_FILEPATH

LIBFFTW_INSTALL_NAME_TOOL_ID = install_name_tool -id @executable_path/../$$LIBSDIR/$$LIBFFTW_FILE \
                $$INSTALLROOT/$$LIBSDIR/$$LIBFFTW_FILE
LIBFFTW_ID.path     = $$INSTALLROOT/$$LIBSDIR
LIBFFTW_ID.commands = $$LIBFFTW_INSTALL_NAME_TOOL_ID
