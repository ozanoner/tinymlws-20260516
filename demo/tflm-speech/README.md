# TFLite Micro Speech example

Keyword-spotting demo using TensorFlow Lite for Microcontrollers on an ESP32-S3.
The model recognises four categories — **silence**, **unknown**, **yes**, and **no** — from 16 kHz PCM audio.

- In offline/simulation mode two 1-second pre-recorded clips (`no_1000ms.wav` and `yes_1000ms.wav`) are fed to the pipeline sequentially through `AppFeed`.
- Each clip is sliced into strides with feature window.
- `AppFeatures` converts each slice into a log-mel spectrogram and accumulates slices into the  input tensor expected by the model.
- `AppInference` runs the int8 TFLite Micro interpreter and logs any detection whose softmax score exceeds 0.80.

## Project structure

```
main/
├── main.cpp                # Entry point — init + run loop
├── AppFeed.hpp             # Audio source: slices pre-recorded PCM into strides
├── AppFeatures.hpp         # Builds log-mel spectrogram from audio slices
├── AppInference.hpp        # Runs TFLite Micro interpreter, logs detections > 0.80
├── data/                   # Pre-recorded WAV clips (no_1000ms, yes_1000ms) as C arrays
└── model/
    ├── model.cc/.h         # Quantised int8 keyword-spotting model
    ├── micro_model_settings.h          # Feature/model constants (sample rate, window, etc.)
    ├── micro_features_generator.cc/.h  # Log-mel feature extraction
    └── ringbuf.c/.h        # Ring buffer helper
```

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
