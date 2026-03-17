#!/bin/sh
export LD_LIBRARY_PATH=../../src
export DYLD_FALLBACK_LIBRARY_PATH=../../src

if [ -z "$QLC_PLUGIN_DIR" ]; then
    export QLC_PLUGIN_DIR=../../../plugins/artnet/src
fi

./universeperf_test -maxwarnings 100000 "$@"
