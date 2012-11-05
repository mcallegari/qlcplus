#!/bin/bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../src
export DYLD_FALLBACK_LIBRARY_PATH=../../src
./script_test
