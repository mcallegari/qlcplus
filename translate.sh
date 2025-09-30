#!/usr/bin/env bash
set -euo pipefail

# Usage:
#   ./translate.sh update
#   ./translate.sh release [qmlui|ui]
#   ./translate.sh create <ll_CC> [qmlui|ui]
#
# Notes:
# - [qmlui|ui] applies ONLY to release and create.
# - update: finds every dir with *.ts and runs:
#           lupdate <that/dir> -ts <that/dir>/*.ts
# - create: for each existing TS basename (e.g., qlcplus from qlcplus_it_IT.ts),
#           runs: lupdate <dir> -ts <dir>/<basename>_<ll_CC>.ts

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

# Helper: detect cmake build dirs (by name or contents)
is_cmake_build_dir() {
  local d="$1"
  [[ "$d" =~ (^|/)(build[^/]*)($|/) ]] || [[ -e "$d/CMakeCache.txt" ]] || [[ -d "$d/CMakeFiles" ]]
}

find_ts_for_lang() {
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
    mapfile -t DIRS < <(
      find . -type f -name "*.ts" -print0 \
      | xargs -0 -n1 dirname \
      | sort -u
    )
    if (( ${#DIRS[@]} == 0 )); then
      echo "No .ts files found."; exit 0
    fi
    shopt -s nullglob
    for d in "${DIRS[@]}"; do
      if is_cmake_build_dir "$d"; then
        echo "Skipping CMake build dir: $d"
        continue
      fi
      ts_in_dir=("$d"/*.ts)
      (( ${#ts_in_dir[@]} == 0 )) && continue
      echo "Updating: $d"
      "$LUPDATE" "$d" -ts "${ts_in_dir[@]}"
    done
    echo "Update complete."
    ;;

  release)
    FLAVOR="${2:-ui}"
    [[ "$FLAVOR" == "ui" || "$FLAVOR" == "qmlui" ]] || die "Flavor must be 'ui' or 'qmlui'"
    LANGS="$UI_LANGS"; [[ "$FLAVOR" == "qmlui" ]] && LANGS="$QMLUI_LANGS"

    for lang in $LANGS; do
      echo "Releasing $lang ($FLAVOR)"
      mapfile -t FILES < <(find_ts_for_lang "$lang" "$FLAVOR")
      if (( ${#FILES[@]} == 0 )); then
        echo "  No TS files for $lang, skipping."
        continue
      fi
      out="qlcplus_${lang}.qm"
      "$LRELEASE" -silent "${FILES[@]}" -qm "$out"
      echo "  -> $out"
    done
    ;;

  create)
    NEW_LANG="${2:-}"; FLAVOR="${3:-ui}"
    [[ -n "$NEW_LANG" ]] || { usage; exit 1; }
    [[ "$FLAVOR" == "ui" || "$FLAVOR" == "qmlui" ]] || die "Flavor must be 'ui' or 'qmlui'"
    [[ "$NEW_LANG" =~ ^[a-z]{2}_[A-Z]{2}$ ]] || die "Language code must be ll_CC (e.g., it_IT, pt_BR)"

    echo "Creating TS files for $NEW_LANG ($FLAVOR) ..."
    # Collect all reference TS files for the selected flavor
    if [[ "$FLAVOR" == "qmlui" ]]; then
      mapfile -t REF_TS < <(find . -type f -path "./qmlui/*" -name "*_[a-z][a-z]_[A-Z][A-Z].ts")
    else
      mapfile -t REF_TS < <(find . -type f -not -path "./qmlui/*" -name "*_[a-z][a-z]_[A-Z][A-Z].ts")
    fi

    if (( ${#REF_TS[@]} == 0 )); then
      echo "No reference .ts files found for flavor '$FLAVOR'."
      echo "Creating a default file in current directory: ./qlcplus_${NEW_LANG}.ts"
      "$LUPDATE" . -ts "./qlcplus_${NEW_LANG}.ts"
      echo "Create complete."
      exit 0
    fi

    # For each distinct (dir, basename), run lupdate <dir> -ts <dir>/<basename>_<NEW_LANG>.ts
    declare -A DONE=()
    created_any=0

    for f in "${REF_TS[@]}"; do
      d="$(dirname "$f")"
      if is_cmake_build_dir "$d"; then
        echo "Skipping CMake build dir: $d"
        continue
      fi
      fname="$(basename "$f")"
      # Strip the trailing _ll_CC.ts to get the basename, robust to underscores in the name
      base="$(printf '%s\n' "$fname" | sed -E 's/_([a-z]{2}_[A-Z]{2})\.ts$//')"
      key="$d|$base"
      [[ -n "${DONE[$key]:-}" ]] && continue
      DONE["$key"]=1

      new_ts="$d/${base}_${NEW_LANG}.ts"
      echo "Generating: $new_ts"
      "$LUPDATE" "$d" -ts "$new_ts"
      created_any=1
    done

    if (( created_any == 0 )); then
      echo "Nothing to create (no basenames discovered)."
    else
      echo "Create complete."
    fi
    ;;

  ""|-h|--help)
    usage
    ;;

  *)
    usage; exit 1
    ;;
esac
