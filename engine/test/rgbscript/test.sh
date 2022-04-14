#!/bin/bash
export LD_LIBRARY_PATH=../../src:$LD_LIBRARY_PATH
export DYLD_FALLBACK_LIBRARY_PATH=../../src
./rgbscript_test
