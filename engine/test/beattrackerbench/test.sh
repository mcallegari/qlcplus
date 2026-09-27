#!/bin/sh
export LD_LIBRARY_PATH=../../src
export DYLD_FALLBACK_LIBRARY_PATH=../../src
# Quick mode: asserts the default tracker on 16 clips and prints the
# three-way comparison. Run ./beattrackerbench --full for the complete
# 8 scenarios x 8 tempi table.
./beattrackerbench
