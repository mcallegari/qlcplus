#!/bin/sh
export LD_LIBRARY_PATH=../../src
export DYLD_FALLBACK_LIBRARY_PATH=../../src
cp qlci18n_fi_FI.ts_ qlci18n_fi_FI.ts
if [ -n "$QTDIR" ]; then
    $QTDIR/bin/lupdate . -ts qlci18n_fi_FI.ts
    $QTDIR/bin/lrelease -silent qlci18n_fi_FI.ts -qm qlci18n_fi_FI.qm
else
    lupdate . -ts qlci18n_fi_FI.ts
    lrelease -silent qlci18n_fi_FI.ts -qm qlci18n_fi_FI.qm
fi
./qlci18n_test
