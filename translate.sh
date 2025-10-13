#!/usr/bin/env bash
set -euo pipefail

# Usage:
#   ./translate.sh update
#   ./translate.sh release [qmlui|ui]
#   ./translate.sh create <ll_CC> [qmlui|ui]
#   ./translate.sh stats

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

STATS_LANGS="de_DE es_ES fr_FR it_IT nl_NL cz_CZ pt_BR ca_ES ja_JP ru_RU uk_UA pl_PL" # merge of both above for stats
EMOJI_FLAGS="🇩🇪 🇪🇸 🇫🇷 🇮🇹 🇳🇱 🇨🇿 🇧🇷 🇪🇸 🇯🇵 🇷🇺 🇺🇦 🇵🇱"

# [path|category_name]
TS_FILES=(
  "./fixtureeditor/fixtureeditor_xx_YY.ts|Fixture Editor"
  "./launcher/launcher_xx_YY.ts|Launcher"
  "./plugins/artnet/src/ArtNet_xx_YY.ts|ArtNet Plugin"
  "./plugins/dmx4linux/DMX4Linux_xx_YY.ts|DMX4Linux Plugin"
  "./plugins/dmxusb/src/DMX_USB_xx_YY.ts|DMX-USB Plugin"
  "./plugins/E1.31/E131_xx_YY.ts|E1.31 Plugin"
  "./plugins/enttecwing/src/ENTTEC_Wing_xx_YY.ts|ENTTEC Wing Plugin"
  "./plugins/gpio/GPIO_xx_YY.ts|GPIO Plugin"
  "./plugins/hid/HID_xx_YY.ts|HID Plugin"
  "./plugins/loopback/src/loopback_xx_YY.ts|Loopback Plugin"
  "./plugins/midi/src/MIDI_xx_YY.ts|MIDI Plugin"
  "./plugins/ola/OLA_xx_YY.ts|OLA Plugin"
  "./plugins/os2l/OS2L_xx_YY.ts|OS2L Plugin"
  "./plugins/osc/OSC_xx_YY.ts|OSC Plugin"
  "./plugins/peperoni/Peperoni_xx_YY.ts|Peperoni Plugin"
  "./plugins/spi/SPI_xx_YY.ts|SPI Plugin"
  "./plugins/uart/UART_xx_YY.ts|UART Plugin"
  "./plugins/udmx/src/uDMX_xx_YY.ts|uDMX Plugin"
  "./plugins/velleman/src/Velleman_xx_YY.ts|Velleman Plugin"
  "./qmlui/qlcplus_xx_YY.ts|QML UI"
  "./ui/src/qlcplus_xx_YY.ts|UI"
  "./webaccess/src/webaccess_xx_YY.ts|Web Access"
)

die() { echo "Error: $*" >&2; exit 1; }
usage() {
  cat <<EOF
Usage:
  $0 update
  $0 release [qmlui|ui]
  $0 create <ll_CC> [qmlui|ui]
  $0 stats
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

compute_translation_percentage() {
  # read .ts file and count total and translated strings
  # simply count <message> and <translation> tags
  # and translation with type="unfinished" attribute
  local ts_file_path="$1"
  local total translated percent

  total=$(awk '/<message/ {c++} END {print c+0}' "$ts_file_path")
  translated=$(awk '/<translation/ && $0 !~ /type="unfinished"/ {c++} END {print c+0}' "$ts_file_path")

  if [[ "$total" -eq 0 ]]; then
    echo 0
    return
  fi

  percent=$(( translated * 100 / total ))
  echo "$percent"
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

  stats)
    header="| Projects |"
    separator="|----------|"

    for flag in $EMOJI_FLAGS; do
      header+=" ${flag} |"
      separator+="-------|"
    done
    echo "$header"
    echo "$separator"

    # parse each ts file
    for entry in "${TS_FILES[@]}"; do
      IFS='|' read -r ts_file category <<<"$entry"
      row="| ${category} |"
      for lang in $STATS_LANGS; do
        ts_file_path="${ts_file//xx_YY/$lang}"
        if [[ -f "$ts_file_path" ]]; then
          percent="$(compute_translation_percentage "$ts_file_path")"
          if [[ "$percent" -ge 100 ]]; then
            row+="   ✅    |"
          elif [[ "$percent" -eq 0 ]]; then
            row+="   ❌    |"
          else
            printf -v cell "  %3d%%  |" "$percent"
            row+="$cell"
          fi
        else
          row+="   ❌    |"
        fi
      done
      echo "$row"
    done
    ;;

  ""|-h|--help)
    usage
    ;;

  *)
    usage; exit 1
    ;;
esac
