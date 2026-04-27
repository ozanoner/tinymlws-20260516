# TFlite Micro Speech example

## Running on Wokwi

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


## References
- https://code.vt.edu/thomaspj1017/tflite-micro/-/tree/main/tensorflow/lite/micro/examples/micro_speech
- https://github.com/espressif/esp-tflite-micro/tree/master/examples/micro_speech
