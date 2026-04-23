# Edge Impulse keyword spotting example

Offline keyword spotting example for this ESP-IDF component.

The app loads a bundled offline audio sample, runs a single inference, logs basic model information, and prints the classifier output.

Note: Use the helper script at [tools/wavtoh.sh](../../tools/wavtoh.sh) to convert a WAV file into a C header for bundled sample audio.

## Running on Wokwi

Build:

```bash
rm -rf sdkconfig build/ && PRJ_BUILD_TARGET=wokwi idf.py build
```

Run:

```bash
wokwi-cli . --timeout 120000 --fail-text "Backtrace:" --expect-text "Returned from app_main"
```

### Running on ESP32-S3

Build:

```bash
rm -rf sdkconfig build/ && PRJ_BUILD_TARGET=esp32s3 idf.py build
```

Run:

```bash
idf.py flash monitor -p /dev/ttyACM0
```

