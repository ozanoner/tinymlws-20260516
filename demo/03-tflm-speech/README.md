# TFLite Micro Speech example

Offline keyword-spotting demo for ESP32-S3 using TensorFlow Lite for Microcontrollers.
The int8 model recognises four categories from 16 kHz PCM audio: **silence**, **unknown**, **yes**, and **no**.

`AppFeed` plays two bundled 1-second clips (`no_1000ms.wav` and `yes_1000ms.wav`) sequentially and serves overlapping audio windows to the feature pipeline.
`AppFeatures` converts each 30 ms window with a 20 ms stride into log-mel features and accumulates a 49 x 40 spectrogram for inference.
`AppInference` runs the TFLite Micro interpreter and logs detections whose score exceeds `0.80`.

Use [tools/wavtoh.sh](/workspaces/tools/wavtoh.sh) to convert a WAV file into a C++ header for additional bundled test clips.

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


## References
- https://www.tensorflow.org/tutorials/audio/simple_audio#run_inference_on_an_audio_file
- https://ai.google.dev/edge/litert/microcontrollers/overview
- https://github.com/tensorflow/tflite-micro/tree/main/tensorflow/lite/micro/examples/micro_speech
- https://github.com/tensorflow/tflite-micro/blob/main/tensorflow/lite/micro/examples/micro_speech/train/train_micro_speech_model.ipynb
- https://github.com/espressif/esp-tflite-micro/tree/master/examples/micro_speech


