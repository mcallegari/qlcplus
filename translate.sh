#!/bin/bash

# Translated languages (update these also to qlcplus.pro)
languages="de_DE es_ES fi_FI fr_FR it_IT nl_NL cz_CZ"

# Compile all files for the given language into one common qlcplus_<lang>.qm file
function compile {
    echo Processing $1
    lrelease -silent `find . -name *_$1.ts` -qm qlcplus_$1.qm
}

# Compile all translated languages present in $languages
for lang in $languages; do
    compile $lang
done

