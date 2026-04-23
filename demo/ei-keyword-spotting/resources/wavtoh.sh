#!/bin/bash
# Converts a WAV file into a C header with 16-bit mono 16 kHz
# PCM samples for use in the example applications.

set -e

# Check arguments
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 input.wav output.h"
    exit 1
fi

INPUT="$1"
OUTPUT="$2"

# Temp file
TMP_RAW=$(mktemp)

# Convert WAV -> raw PCM (16-bit, mono, 16kHz)
ffmpeg -y -i "$INPUT" -f s16le -acodec pcm_s16le -ac 1 -ar 16000 "$TMP_RAW" 

# Generate header file
{
    echo "static const int16_t kOfflineKeywordSample[] = {"

    hexdump -v -e '1/2 "%d,\n"' "$TMP_RAW"

    echo "};"
    echo ""
    echo "static const int kOfflineKeywordSampleLength = sizeof(kOfflineKeywordSample)/sizeof(kOfflineKeywordSample[0]);"
} > "$OUTPUT"

# Cleanup
rm "$TMP_RAW"

echo "Generated $OUTPUT successfully."