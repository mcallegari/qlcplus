#!/bin/sh
export QT_QPA_PLATFORM=offscreen
export LD_LIBRARY_PATH=../../../engine/src:$LD_LIBRARY_PATH
export DYLD_FALLBACK_LIBRARY_PATH=../../../engine/src:$DYLD_FALLBACK_LIBRARY_PATH
./vcwidget_test
