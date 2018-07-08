#!/bin/sh
export LD_LIBRARY_PATH=../../src
export DYLD_FALLBACK_LIBRARY_PATH=../../src
cp qlci18n_fi_FI.ts_ qlci18n_fi_FI.ts
if [ -n "$QTDIR" ]; then
    $QTDIR/bin/lupdate qlci18n.pro
    $QTDIR/bin/lrelease qlci18n.pro
else
    lupdate qlci18n.pro
    lrelease qlci18n.pro
fi
./qlci18n_test
