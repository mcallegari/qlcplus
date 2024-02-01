#!/bin/bash

THISCMD=`basename "$0"`
THISDIR=`dirname "$0"`
DIFFARG="-q"
CHECK_MODE="yes"
INDENT_MODE="no"
CLANG_FORMAT=clang-format-14

TARGET="qmlui"
MODE="default"

cd "$THISDIR"

usage() {
  echo >&2 "$THISCMD [options] [indentPathOrFile ...]"
  echo >&2 "-c|--check            Script mode: check (default)"
  echo >&2 "-h|--help             Print this help message"
  echo >&2 "-i|--indent           Script mode: indent"
  echo >&2 "-t|--target target    Build Target (qmlui|ui) default: qmlui"
  echo >&2 "-u|--unify            Add unified diff output"
}

if [ -z `which "$CLANG_FORMAT"` ]; then
  echo >&2 "$CLANG_FORMAT: Error: Program not found."
  exit 1
fi

ERRORS=0
SEARCHPATH=()
while [[ $# -gt 0 ]]; do
  case "$1" in
    -t|--target)
      TARGET="$2"
      case "$TARGET" in
        ui|qmlui)
        ;;
        *)
          echo "Unknown target option $2"
          ERRORS=1
          ;;
      esac
      shift # past argument
      shift # past value
      ;;
    -i|--indent)
      MODE="indent"
      INDENT_MODE="yes"
      shift # past argument
      ;;
    -c|--check)
      MODE="check"
      CHECK_MODE="yes"
      shift # past argument
      ;;
    -u|--unified)
      DIFFARG="-u"
      shift # past argument
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    -*|--*)
      echo "Unknown option $1"
      shift # past argument
      ERRORS=1
      ;;
    *)
      if [ -r "$1" ]; then
        SEARCHPATH+=("$1")
      else
        echo >&2 "File or directory not readable: $1"
        ERRORS=1
      fi
      shift # past argument
      ;;
  esac
done

if [ $ERRORS -ne 0 ]; then
  exit $ERRORS
fi

# Fall back to check mode if no mode is set
if [ $MODE == "default" ]; then
  CHECK_MODE="yes"
fi

if [ ${#SEARCHPATH[@]} -eq 0 ]; then 
  ### Excluded directories
  #    plugins \
  #    fixtureeditor \
  #    hotplugmonitor \

  # Common directories
  SEARCHPATH+=("engine")
  SEARCHPATH+=("launcher")
  SEARCHPATH+=("resources/rgbscripts")

  if [ "$TARGET" == "ui"]; then
    SEARCHPATH+=("ui")
    SEARCHPATH+=("webaccess")
  else
    # fallback to add qmlui directories (default mode)
    SEARCHPATH+=("qmlui")
  fi
fi

TMPFILE=`mktemp /tmp/$THISCMD.XXXXXXXX`
if [ ! -f "$TMPFILE" ]; then
  echo >&2 "Unable to create temporary file."
  exit 1
fi

echo >&2 "Processed paths:"
for i in ${SEARCHPATH[@]}; do
  echo >&2 "  $i"
done

#############################################################################
# Indentation application
#############################################################################

if [ "$INDENT_MODE" == "yes" ]; then
  # CPP code indentation
  find \
      ${SEARCHPATH[@]} \
      -name '*.h' -and -not -name 'moc_*' -and -not -name 'ui_*' -and -not -name 'qlcconfig.h' -or \
      -name '*.cpp' -and -not -name 'moc_*' -and -not -name 'qrc_*' | while read FILE; do
    $CLANG_FORMAT -i -style=file:.clang-format "$FILE"
  done

  # JS code indentation
  find \
      ${SEARCHPATH[@]} \
    -name '*.js' | while read FILE; do
    $CLANG_FORMAT -i -style=file:.clang-format-js "$FILE"
  done
fi

#############################################################################
# Indentation check
#############################################################################

if [ "$CHECK_MODE" == "yes" ]; then
  # CPP code indentation check
  find \
      ${SEARCHPATH[@]} \
      -name '*.h' -and -not -name 'moc_*' -and -not -name 'ui_*' -and -not -name 'qlcconfig.h' -or \
      -name '*.cpp' -and -not -name 'moc_*' -and -not -name 'qrc_*' | while read FILE; do
    $CLANG_FORMAT -style=file:.clang-format "$FILE" | diff $DIFFARG "$FILE" -
    RET=$?
    if [ $RET -ne 0 ]; then
      #echo >&2 "$FILE: Error in formatting."
      echo "  $FILE" >> "$TMPFILE"
    #else
      #echo >&2 "$FILE: Formatting OK"
    fi
  done

  # JS code indentation check
  find \
      ${SEARCHPATH[@]} \
      -name '*.js' | while read FILE; do
    $CLANG_FORMAT -style=file:.clang-format-js "$FILE" | diff $DIFFARG "$FILE" -
    RET=$?
    if [ $RET -ne 0 ]; then
      #echo >&2 "$FILE: Error in formatting."
      echo "  $FILE" >> "$TMPFILE"
    #else
      #echo >&2 "$FILE: Formatting OK"
    fi
  done
fi

ERRCODE=0
if [ -s "$TMPFILE" ]; then
  echo >&2
  echo >&2 "Error in formatting. Run:"
  echo >&2 "  $THISCMD -i <file>"
  echo >&2 "on following files:"
  cat "$TMPFILE"
  ERRCODE=1
fi
rm "$TMPFILE"
exit $ERRCODE

