
#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_PATH="$(realpath "$SCRIPT_DIR/..")"

UI_LANGS="de_DE es_ES fr_FR it_IT nl_NL cz_CZ pt_BR ca_ES ja_JP ru_RU uk_UA pl_PL"
EMOJI_FLAGS="🇩🇪 🇪🇸 🇫🇷 🇮🇹 🇳🇱 🇨🇿 🇧🇷 🇪🇸 🇯🇵 🇷🇺 🇺🇦 🇵🇱"

# [path|category_name]
TS_FILES=(
  "$ROOT_PATH/fixtureeditor/fixtureeditor_xx_YY.ts|Fixture Editor"
  "$ROOT_PATH/launcher/launcher_xx_YY.ts|Launcher"
  "$ROOT_PATH/plugins/artnet/src/ArtNet_xx_YY.ts|ArtNet Plugin"
  "$ROOT_PATH/plugins/dmx4linux/DMX4Linux_xx_YY.ts|DMX4Linux Plugin"
  "$ROOT_PATH/plugins/dmxusb/src/DMX_USB_xx_YY.ts|DMX-USB Plugin"
  "$ROOT_PATH/plugins/E1.31/E131_xx_YY.ts|E1.31 Plugin"
  "$ROOT_PATH/plugins/enttecwing/src/ENTTEC_Wing_xx_YY.ts|ENTTEC Wing Plugin"
  "$ROOT_PATH/plugins/gpio/GPIO_xx_YY.ts|GPIO Plugin"
  "$ROOT_PATH/plugins/hid/HID_xx_YY.ts|HID Plugin"
  "$ROOT_PATH/plugins/loopback/src/loopback_xx_YY.ts|Loopback Plugin"
  "$ROOT_PATH/plugins/midi/src/MIDI_xx_YY.ts|MIDI Plugin"
  "$ROOT_PATH/plugins/ola/OLA_xx_YY.ts|OLA Plugin"
  "$ROOT_PATH/plugins/os2l/OS2L_xx_YY.ts|OS2L Plugin"
  "$ROOT_PATH/plugins/osc/OSC_xx_YY.ts|OSC Plugin"
  "$ROOT_PATH/plugins/peperoni/Peperoni_xx_YY.ts|Peperoni Plugin"
  "$ROOT_PATH/plugins/spi/SPI_xx_YY.ts|SPI Plugin"
  "$ROOT_PATH/plugins/uart/UART_xx_YY.ts|UART Plugin"
  "$ROOT_PATH/plugins/udmx/src/uDMX_xx_YY.ts|uDMX Plugin"
  "$ROOT_PATH/plugins/velleman/src/Velleman_xx_YY.ts|Velleman Plugin"
  "$ROOT_PATH/qmlui/qlcplus_xx_YY.ts|QML UI"
  "$ROOT_PATH/ui/src/qlcplus_xx_YY.ts|UI"
  "$ROOT_PATH/webaccess/src/webaccess_xx_YY.ts|Web Access"
)

parse_ts_file() {
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

main() {
  # print header
  local header="| Projects |"
  local separator="|----------|"
  local flag

  for flag in $EMOJI_FLAGS; do
    header+=" ${flag} |"
    separator+="-------|"
  done
  echo "$header"
  echo "$separator"

  # parse each ts file
  local entry ts_file category row lang ts_file_path percent cell
  for entry in "${TS_FILES[@]}"; do
    IFS='|' read -r ts_file category <<<"$entry"
    row="| ${category} |"
    for lang in $UI_LANGS; do
      ts_file_path="${ts_file//xx_YY/$lang}"
      if [[ -f "$ts_file_path" ]]; then
        percent="$(parse_ts_file "$ts_file_path")"
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
}

if [[ "${BASH_SOURCE[0]}" == "$0" ]]; then
  main "$@"
fi
