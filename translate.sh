#!/bin/bash

# Translated languages (update these also to qlc.pro)
languages="de_DE es_ES fi_FI fr_FR it_IT"

# Compile all files for the given language into one common qlc_<lang>.qm file
function compile {
    for file in `find . -name "*_$1.ts"`; do
        tsfiles="$tsfiles $file"
    done

    echo Processing $1
    lrelease -silent $tsfiles -qm qlc_$1.qm
}

# Compile all translated languages present in $languages
for lang in $languages; do
    compile $lang
done

