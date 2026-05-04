# Wake-word detection with ESP-SR

Offline wake-word detection demo using Espressif ESP-SR WakeNet.
`AppFeed` streams pre-recorded PCM audio (`data/hiesp.h`) in model-sized chunks,
`AppInference` loads the `wn9s_hiesp` model from the `model` partition, runs
`wakenet->detect(...)`, and logs `Wake word detected!` when the trigger fires.

## Project structure

```text
main/
├── main.cpp             # Entry point: init feed/inference and run detect loop
├── AppFeed.hpp          # Feeds chunked PCM audio from data/hiesp.h
└── AppInference.hpp     # Loads WakeNet model and performs wake-word detection

components/
├── hardware_driver/     # Board/audio hardware abstraction used by ESP-SR stack
├── perf_tester/         # Optional performance measurement helpers
├── player/              # Audio playback component utilities
└── sr_ringbuf/          # Ring buffer support for speech/audio pipelines

data/
└── hiesp.h              # Bundled offline PCM sample used as input audio
```

## Running on Wokwi

Note: It runs on board-esp32-devkit-c-v4, NOT an ESP32-S3 device.

Build:

```bash
rm -rf sdkconfig build/ && PRJ_TARGET=wokwi idf.py build
```

Run:

```bash
wokwi-cli . --timeout 120000 --fail-text "Backtrace:" --expect-text "Returned from app_main"
```

## Running on ESP32-S3

Build:

```bash
rm -rf sdkconfig build/ && PRJ_TARGET=esp32s3 idf.py build
```

Run:

```bash
idf.py flash monitor -p /dev/ttyACM0
```

## References:
- https://docs.espressif.com/projects/esp-sr/en/latest/esp32s3/wake_word_engine/README.html
- https://docs.espressif.com/projects/esp-sr/en/latest/esp32s3/audio_front_end/README.html
- https://docs.espressif.com/projects/esp-sr/en/latest/esp32s3/speech_command_recognition/README.html
- https://docs.espressif.com/projects/esp-sr/en/latest/esp32s3/benchmark/README.html
- https://github.com/espressif/esp-skainet/tree/master
