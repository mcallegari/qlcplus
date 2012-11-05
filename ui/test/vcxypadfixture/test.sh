#!/bin/sh
LD_LIBRARY_PATH=../../src:../../../engine/src \
    DYLD_FALLBACK_LIBRARY_PATH=../../src:../../../engine/src \
    ./vcxypadfixture_test
