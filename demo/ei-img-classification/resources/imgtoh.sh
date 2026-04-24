#!/usr/bin/env bash

set -euo pipefail

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <input_file> <output_header>" >&2
    exit 1
fi

INPUT="$1"
OUTPUT="$2"

OUTDIR="$(dirname "$OUTPUT")"
OUTBASE="$(basename "$OUTPUT")"
OUTSTEM="${OUTBASE%.*}"
RESIZED_JPG="$OUTDIR/$OUTSTEM.jpg"

TMP_RAW="$(mktemp /tmp/imgtoh_raw_XXXXXX)"
trap 'rm -f "$TMP_RAW"' EXIT

if ! command -v ffmpeg >/dev/null 2>&1; then
    echo "Error: required command 'ffmpeg' not found in PATH" >&2
    exit 1
fi

if ! command -v od >/dev/null 2>&1; then
    echo "Error: required command 'od' not found in PATH" >&2
    exit 1
fi

mkdir -p "$OUTDIR"

# Resize to 96x96 JPEG
ffmpeg -loglevel error -y -i "$INPUT" \
    -vf "scale=96:96:flags=lanczos" \
    -frames:v 1 \
    "$RESIZED_JPG"

# Extract raw RGB24 bytes
ffmpeg -loglevel error -y -i "$RESIZED_JPG" \
    -f rawvideo -pix_fmt rgb24 \
    -frames:v 1 \
    "$TMP_RAW"

{
    echo "#pragma once"
    echo
    echo "#include <stddef.h>"
    echo "#include <stdint.h>"
    echo
    echo "static const uint32_t features[] = {"

    od -An -v -t u1 "$TMP_RAW" | \
    tr -s '[:space:]' '\n' | \
    awk '
        /^[0-9]+$/ {
            buf[count++] = $1
            if (count == 3) {
                printf "    0x%02x%02x%02x,\n", buf[0], buf[1], buf[2]
                count = 0
            }
        }
    '

    echo "};"
    echo
} > "$OUTPUT"

echo "Generated: $OUTPUT"
echo "Generated: $RESIZED_JPG"
echo "Feature count: $(grep -c '0x' "$OUTPUT")"