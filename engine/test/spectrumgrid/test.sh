#!/bin/sh
export LD_LIBRARY_PATH=../../audio/src
export DYLD_FALLBACK_LIBRARY_PATH=../../audio/src
./spectrumgrid_test
