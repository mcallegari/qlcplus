#!/bin/sh
export LD_LIBRARY_PATH=../../src
export DYLD_FALLBACK_LIBRARY_PATH=../../src
./grandmaster_test -iterations 5000
