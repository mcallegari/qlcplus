#!/bin/sh
export LD_LIBRARY_PATH=../../src
export DYLD_FALLBACK_LIBRARY_PATH=../../src
lupdate qlci18n.pro
lrelease qlci18n.pro
./qlci18n_test
