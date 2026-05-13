#!/usr/bin/env bash

# Converts an image to a 96x96 C/C++ header payload, optionally in grayscale (-g).
# Usage: ./imgtoh.sh <input_file> <output_file> [-g]
# Example: ./imgtoh.sh face1.png face1.h -g

set -euo pipefail

GRAYSCALE=0
if [ "$#" -eq 3 ] && [ "$3" = "-g" ]; then
    GRAYSCALE=1
elif [ "$#" -ne 2 ]; then
    echo "Usage: $0 <input_file> <output_file> [-g]" >&2
    echo "  -g  Output grayscale C++ header (for tflm-person-detection)" >&2
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

generate_header() {
    local grayscale="$1"

    echo "#pragma once"
    echo
    echo "#include <cstdint>"

    if [ "$grayscale" -eq 0 ]; then
        echo "#include <cstddef>"
    fi

    echo

    if [ "$grayscale" -eq 1 ]; then
        echo "static const uint8_t image_data[] = {"
        od -An -v -t u1 "$TMP_RAW" | \
        tr -s '[:space:]' '\n' | \
        awk '/^[0-9]+$/ { printf "    0x%02x,\n", $1 }'
        echo "};"
        echo
        echo "static const int image_data_length = $(wc -c < "$TMP_RAW");"
    else
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
    fi
    echo
}

# Resize to 96x96 JPEG
ffmpeg -loglevel error -y -i "$INPUT" \
    -vf "scale=96:96:flags=lanczos" \
    -frames:v 1 \
    "$RESIZED_JPG"

# Extract raw pixels (grayscale or RGB24)
PIX_FMT="rgb24"
if [ "$GRAYSCALE" -eq 1 ]; then
    PIX_FMT="gray"
fi

ffmpeg -loglevel error -y -i "$RESIZED_JPG" \
    -f rawvideo -pix_fmt "$PIX_FMT" \
    -frames:v 1 \
    "$TMP_RAW"

generate_header "$GRAYSCALE" > "$OUTPUT"

echo "Generated: $OUTPUT"
echo "Generated: $RESIZED_JPG"
echo "Feature count: $(grep -c '0x' "$OUTPUT")"