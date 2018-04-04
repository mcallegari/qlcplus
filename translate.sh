#!/bin/bash

# Translated languages (update these also to qlcplus.pro)
languages="de_DE es_ES fr_FR it_IT nl_NL cz_CZ pt_BR ca_ES ja_JP"

if [ -n "$1" ]; then
    echo "Forcing the use of lrelease-$1"
    LRELEASE_BIN=$(which lrelease-$1)
else
    LRELEASE_BIN=$(which lrelease)
fi

# if QTDIR has been defined, use those tools right away
if [ -n "$QTDIR" ]; then
    LRELEASE_BIN=$QTDIR/bin/lrelease
else
    # if lrelease is not available, try with lrelease-qt4
    if [ -z "$LRELEASE_BIN" ]; then
        LRELEASE_BIN=$(which lrelease-qt4)

        # if lrelease-qt4 is not available, try with lrelease-qt5
        if [ -z "$LRELEASE_BIN" ]; then
	    LRELEASE_BIN=$(which lrelease-qt5)
	    if [ -z "$LRELEASE_BIN" ]; then
		echo "lrelease, lrelease-qt4 and lrelease-qt5 are not present in this system ! Aborting."
		exit
	    fi
        fi
    fi
fi

# Compile all files for the given language into one common qlcplus_<lang>.qm file
function compile {
    echo Processing $1
    INPUT_NAME="*_${1}.ts"
    OUTPUT_NAME="qlcplus_${1}.qm"
    $LRELEASE_BIN -silent $(find . -name $INPUT_NAME) -qm $OUTPUT_NAME
}

# Compile all translated languages present in $languages
for lang in $languages; do
    compile $lang
done

