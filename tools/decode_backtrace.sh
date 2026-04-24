#!/usr/bin/env bash
set -euo pipefail

TARGET="${1:-${PRJ_BUILD_TARGET:-esp32}}"

case "$TARGET" in
  esp32)   ADDR2LINE="xtensa-esp32-elf-addr2line" ;;
  esp32s2) ADDR2LINE="xtensa-esp32s2-elf-addr2line" ;;
  esp32s3) ADDR2LINE="xtensa-esp32s3-elf-addr2line" ;;
  esp32c2|esp32c3|esp32c6|esp32h2) ADDR2LINE="riscv32-esp-elf-addr2line" ;;
  *) ADDR2LINE="xtensa-esp32-elf-addr2line" ;;
esac

if [[ ! -f CMakeLists.txt ]]; then
  echo "Error: CMakeLists.txt not found in current folder" >&2
  exit 1
fi

PROJECT_NAME="$(
  grep -iE '^[[:space:]]*project[[:space:]]*\(' CMakeLists.txt \
  | head -n1 \
  | sed -E 's/^[[:space:]]*project[[:space:]]*\([[:space:]]*//I; s/[[:space:]]*\).*//; s/^"//; s/"$//'
)"

if [[ -z "${PROJECT_NAME:-}" ]]; then
  echo "Error: could not find project(name) in CMakeLists.txt" >&2
  exit 1
fi

ELF="build/${PROJECT_NAME}.elf"

if [[ ! -f "$ELF" ]]; then
  echo "Error: ELF file not found: $ELF" >&2
  exit 1
fi

if ! command -v "$ADDR2LINE" >/dev/null 2>&1; then
  echo "Error: addr2line tool not found: $ADDR2LINE" >&2
  exit 1
fi

INPUT="$(cat)"
if [[ -z "$INPUT" ]]; then
  echo "Error: no input received on stdin" >&2
  exit 1
fi

mapfile -t ADDRS < <(
  printf '%s\n' "$INPUT" \
  | grep -oE '0x[0-9a-fA-F]+:0x[0-9a-fA-F]+' \
  | cut -d: -f1
)

if [[ ${#ADDRS[@]} -eq 0 ]]; then
  echo "Error: no backtrace addresses found in stdin" >&2
  exit 1
fi

# Colors
if [[ -t 1 ]]; then
  C_RESET=$'\033[0m'
  C_BOLD=$'\033[1m'
  C_DIM=$'\033[2m'
  C_RED=$'\033[31m'
  C_GREEN=$'\033[32m'
  C_YELLOW=$'\033[33m'
  C_BLUE=$'\033[34m'
  C_MAGENTA=$'\033[35m'
  C_CYAN=$'\033[36m'
else
  C_RESET=""
  C_BOLD=""
  C_DIM=""
  C_RED=""
  C_GREEN=""
  C_YELLOW=""
  C_BLUE=""
  C_MAGENTA=""
  C_CYAN=""
fi

PROJECT_ROOT="$(pwd -P)"
IDF_PATH_VAL="${IDF_PATH:-}"
HOME_VAL="${HOME:-}"

pretty_path() {
  local p="$1"

  if [[ -n "$PROJECT_ROOT" && "$p" == "$PROJECT_ROOT/"* ]]; then
    printf '%s\n' "./${p#$PROJECT_ROOT/}"
    return
  fi

  if [[ -n "$IDF_PATH_VAL" && "$p" == "$IDF_PATH_VAL/"* ]]; then
    printf '%s\n' "\$IDF_PATH/${p#$IDF_PATH_VAL/}"
    return
  fi

  if [[ -n "$HOME_VAL" && "$p" == "$HOME_VAL/"* ]]; then
    printf '%s\n' "~/${p#$HOME_VAL/}"
    return
  fi

  printf '%s\n' "$p"
}

print_header() {
  printf "%sTarget%s      : %s%s%s\n"   "$C_CYAN" "$C_RESET" "$C_BOLD" "$TARGET" "$C_RESET"
  printf "%sAddr2line%s   : %s%s%s\n"   "$C_CYAN" "$C_RESET" "$C_BOLD" "$ADDR2LINE" "$C_RESET"
  printf "%sProject%s     : %s%s%s\n"   "$C_CYAN" "$C_RESET" "$C_BOLD" "$PROJECT_NAME" "$C_RESET"
  printf "%sELF%s         : %s%s%s\n"   "$C_CYAN" "$C_RESET" "$C_BOLD" "$ELF" "$C_RESET"
  echo
}

print_header

frame_no=0

for addr in "${ADDRS[@]}"; do
  line="$("$ADDR2LINE" -pfiaC -e "$ELF" "$addr" 2>/dev/null | tr -d '\r')"

  # Typical format:
  # 0x4200d634: app_main at /path/to/file.cpp:66
  # or sometimes:
  # 0x....: ?? ??:0

  address="${line%%:*}"
  rest="${line#*: }"

  func="$rest"
  fileline=""

  if [[ "$rest" == *" at "* ]]; then
    func="${rest% at *}"
    fileline="${rest##* at }"
  fi

  file=""
  lineno=""

  if [[ -n "$fileline" && "$fileline" == *:* ]]; then
    file="${fileline%:*}"
    lineno="${fileline##*:}"
    file="$(pretty_path "$file")"
  fi

  printf "%s#%-2d%s %s%-12s%s " \
    "$C_YELLOW" "$frame_no" "$C_RESET" \
    "$C_GREEN" "$address" "$C_RESET"

  printf "%s%s%s\n" "$C_BOLD" "$func" "$C_RESET"

  if [[ -n "$file" ]]; then
    printf "    %sat%s %s%s%s" "$C_DIM" "$C_RESET" "$C_BLUE" "$file" "$C_RESET"
    if [[ -n "$lineno" ]]; then
      printf "%s:%s%s%s" "$C_MAGENTA" "$C_RESET" "$C_MAGENTA" "$lineno"
      printf "%s" "$C_RESET"
    fi
    printf "\n"
  fi

  frame_no=$((frame_no + 1))
done
