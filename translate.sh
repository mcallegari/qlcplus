#!/usr/bin/env bash
set -euo pipefail

# Usage:
#   ./translate.sh update
#   ./translate.sh release [qmlui|ui]
#   ./translate.sh create <ll_CC> [qmlui|ui]

which_qt() {
  local base="$1"
  command -v "$base" 2>/dev/null || command -v "${base}-qt6" 2>/dev/null || command -v "${base}-qt5" 2>/dev/null || true
}
LRELEASE="$(which_qt lrelease)"
LUPDATE="$(which_qt lupdate)"
: "${LRELEASE:?lrelease not found}"
: "${LUPDATE:?lupdate not found}"

ACTION="${1:-}"
UI_LANGS="de_DE es_ES fr_FR it_IT nl_NL cz_CZ pt_BR ca_ES ja_JP"
QMLUI_LANGS="de_DE es_ES fr_FR it_IT nl_NL ru_RU ca_ES ja_JP uk_UA pl_PL"

die() { echo "Error: $*" >&2; exit 1; }
usage() {
  cat <<EOF
Usage:
  $0 update
  $0 release [qmlui|ui]
  $0 create <ll_CC> [qmlui|ui]
EOF
}

is_cmake_build_dir() {
  # $1 = dir
  case "$1" in
    *"/build" | *"/build/"* | *"/build-"* ) return 0 ;;
  esac
  [[ -e "$1/CMakeCache.txt" || -d "$1/CMakeFiles" ]]
}

find_ts_for_lang() {
  # args: lang, flavor
  local lang="$1"; local flavor="$2"
  if [[ "$flavor" == "qmlui" ]]; then
    find . -type f -path "./qmlui/*" -name "*_${lang}.ts"
  else
    find . -type f -not -path "./qmlui/*" -name "*_${lang}.ts"
  fi
}

case "$ACTION" in
  update)
    echo "Scanning for translation folders and updating .ts files..."
    # Build a unique list of dirs that contain *.ts
    find . -type f -name "*.ts" -print0 \
    | while IFS= read -r -d '' f; do
        dirname "$f"
      done \
    | sort -u \
    | while IFS= read -r d; do
        # skip CMake build dirs
        if is_cmake_build_dir "$d"; then
          echo "Skipping CMake build dir: $d"
          continue
        fi
        # expand .ts in this dir (handle empty via test)
        ts_found=0
        for tsf in "$d"/*.ts; do
          if [[ -e "$tsf" ]]; then ts_found=1; break; fi
        done
        [[ $ts_found -eq 0 ]] && continue
        echo "Updating: $d"
        "$LUPDATE" "$d" -ts "$d"/*.ts
      done
    echo "Update complete."
    ;;

  release)
    FLAVOR="${2:-ui}"
    [[ "$FLAVOR" == "ui" || "$FLAVOR" == "qmlui" ]] || die "Flavor must be 'ui' or 'qmlui'"
    LANGS="$UI_LANGS"; [[ "$FLAVOR" == "qmlui" ]] && LANGS="$QMLUI_LANGS"

    echo "$LANGS" | tr ' ' '\n' | while IFS= read -r lang; do
      echo "Releasing $lang ($FLAVOR)"
      # Gather files
      files_tmp="$(mktemp)"
      if [[ "$FLAVOR" == "qmlui" ]]; then
        find . -type f -path "./qmlui/*" -name "*_${lang}.ts" > "$files_tmp"
      else
        find . -type f -not -path "./qmlui/*" -name "*_${lang}.ts" > "$files_tmp"
      fi
      if ! [ -s "$files_tmp" ]; then
        echo "  No TS files for $lang, skipping."
        rm -f "$files_tmp"
        continue
      fi
      out="qlcplus_${lang}.qm"
      # shellcheck disable=SC2046
      "$LRELEASE" -silent $(cat "$files_tmp") -qm "$out"
      echo "  -> $out"
      rm -f "$files_tmp"
    done
    ;;

  create)
    NEW_LANG="${2:-}"; FLAVOR="${3:-ui}"
    [[ -n "$NEW_LANG" ]] || { usage; exit 1; }
    [[ "$FLAVOR" == "ui" || "$FLAVOR" == "qmlui" ]] || die "Flavor must be 'ui' or 'qmlui'"
    [[ "$NEW_LANG" =~ ^[a-z]{2}_[A-Z]{2}$ ]] || die "Language code must be ll_CC (e.g., it_IT, pt_BR)"

    echo "Creating TS files for $NEW_LANG ($FLAVOR) ..."

    # Collect reference TS files for the flavor
    refs_tmp="$(mktemp)"
    if [[ "$FLAVOR" == "qmlui" ]]; then
      find . -type f -path "./qmlui/*" -name "*_[a-z][a-z]_[A-Z][A-Z].ts" > "$refs_tmp"
    else
      find . -type f -not -path "./qmlui/*" -name "*_[a-z][a-z]_[A-Z][A-Z].ts" > "$refs_tmp"
    fi

    if ! [ -s "$refs_tmp" ]; then
      echo "No reference .ts files found for flavor '$FLAVOR'."
      echo "Creating default: ./qlcplus_${NEW_LANG}.ts"
      "$LUPDATE" . -ts "./qlcplus_${NEW_LANG}.ts"
      echo "Create complete."
      rm -f "$refs_tmp"
      exit 0
    fi

    # Build unique (dir|basename) pairs, stripping the _ll_CC suffix
    pairs_tmp="$(mktemp)"
    while IFS= read -r f; do
      d="$(dirname "$f")"
      fname="$(basename "$f")"
      base="$(printf '%s\n' "$fname" | sed -E 's/_([a-z]{2}_[A-Z]{2})\.ts$//')"
      printf '%s|%s\n' "$d" "$base"
    done < "$refs_tmp" | sort -u > "$pairs_tmp"

    created_any=0
    while IFS='|' read -r d base; do
      # skip CMake build dirs
      if is_cmake_build_dir "$d"; then
        echo "Skipping CMake build dir: $d"
        continue
      fi
      new_ts="$d/${base}_${NEW_LANG}.ts"
      echo "Generating: $new_ts"
      "$LUPDATE" "$d" -ts "$new_ts"
      created_any=1
    done < "$pairs_tmp"

    [[ $created_any -eq 0 ]] && echo "Nothing to create (no basenames discovered)."
    echo "Create complete."
    rm -f "$refs_tmp" "$pairs_tmp"
    ;;

  ""|-h|--help)
    usage
    ;;

  *)
    usage; exit 1
    ;;
esac
