#!/bin/bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../engine/src
export DYLD_FALLBACK_LIBRARY_PATH=../engine/src
./qlcplus-fixturetool "$@"

