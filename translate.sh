#!/bin/bash

# Translated languages (update these also to qlcplus.pro)
if [ "$1" == "qmlui" ]; then
    languages="de_DE es_ES fr_FR it_IT nl_NL ru_RU ca_ES ja_JP uk_UA pl_PL"
else
    languages="de_DE es_ES fr_FR it_IT nl_NL cz_CZ pt_BR ca_ES ja_JP"
fi

# Check for lrelease, lrelease-qt5, or lrelease-qt6
LRELEASE_BIN=$(which lrelease 2> /dev/null)

# If QTDIR is defined, use the tools from that directory
if [ -n "$QTDIR" ]; then
    LRELEASE_BIN=$QTDIR/bin/lrelease
else
    # Fallback to lrelease-qt5 if lrelease is not found
    if [ -z "$LRELEASE_BIN" ]; then
        LRELEASE_BIN=$(which lrelease-qt5 2> /dev/null)
    fi
    
    # Fallback to lrelease-qt6 if lrelease-qt5 is not found
    if [ -z "$LRELEASE_BIN" ]; then
        LRELEASE_BIN=$(which lrelease-qt6 2> /dev/null)
    fi

    # Abort if none are found
    if [ -z "$LRELEASE_BIN" ]; then
        echo "lrelease, lrelease-qt5, and lrelease-qt6 are not present in this system! Aborting."
        exit
    fi
fi

# Compile all files for the given language into one common qlcplus_<lang>.qm file
function compile {
    echo Processing $1
    INPUT_NAME="*_${1}.ts"
    OUTPUT_NAME="qlcplus_${1}.qm"
    if [ "$2" == "qmlui" ]; then
      FILELIST=$(find . -name $INPUT_NAME -not -path "./ui/*" -not -path "./fixtureeditor/*" -not -path "./launcher/*" -not -path "./webaccess/*")
    else
      FILELIST=$(find . -name $INPUT_NAME -not -path "./qmlui/*")
    fi
    #echo $FILELIST
    $LRELEASE_BIN -silent $FILELIST -qm $OUTPUT_NAME
}

# Compile all translated languages present in $languages
for lang in $languages; do
    compile $lang $1
done

