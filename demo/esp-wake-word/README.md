# Wake-word detection with ESP-SR 


## Running on Wokwi

Note: It runs on board-esp32-devkit-c-v4, NOT an ESP32-S3 device.

Build:

```bash
rm -rf sdkconfig build/ && PRJ_BUILD_TARGET=wokwi idf.py build
```

Run:

```bash
wokwi-cli . --timeout 120000 --fail-text "Backtrace:" --expect-text "Returned from app_main"
```

## Running on ESP32-S3

Build:

```bash
rm -rf sdkconfig build/ && PRJ_BUILD_TARGET=esp32s3 idf.py build
```

Run:

```bash
idf.py flash monitor -p /dev/ttyACM0
```

