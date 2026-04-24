# Edge Impulse image classification example

Offline image classification example for this ESP-IDF component.

The app loads a bundled test image, runs a single inference, logs memory usage and timing information, and reports the top prediction.

## Notes

- Impulse: https://studio.edgeimpulse.com/studio/904830/impulse/1/deployment
- Use [tools/imgtoh.sh](../../tools/imgtoh.sh) to resize an input image to 96x96 and convert it into a C header for bundled test data.

## Build and run

If `idf.py` is unavailable, source `~/.espressif/v5.5.4/esp-idf/export.sh` first.

Note: Default build (ie. when no `PRJ_BUILD_TARGET` is provided) is Wokwi.

### Wokwi

Build:

```bash
rm -rf sdkconfig build/ && PRJ_BUILD_TARGET=wokwi idf.py build
```

Run:

```bash
wokwi-cli . --timeout 150000 --fail-text "Backtrace:" --expect-text "main_task: Returned from app_main()"
```

### ESP32-S3 hardware

Build:

```bash
rm -rf sdkconfig build/ && PRJ_BUILD_TARGET=esp32s3 idf.py build
```

Run:

```bash
idf.py flash monitor
```
