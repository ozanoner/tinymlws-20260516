# Edge Impulse keyword spotting example

Offline keyword-spotting demo for ESP32-S3 using a model trained in [Edge Impulse](https://studio.edgeimpulse.com/studio/972337/) (project *tinyml-workshop-kws#1*).
The model classifies 16 kHz audio into two categories: **good_morning** and **unknown**.

`AppFeed` iterates over two bundled 1-second PCM clips (`gm_sample` and `unknown`). For each clip `AppInference` calls `ei_run_classifier` from the Edge Impulse C++ SDK, then prints the label scores via `ei_print_results`.

> **Tip:** use the helper script at [tools/wavtoh.sh](../../tools/wavtoh.sh) to convert a WAV file into a C header for a new bundled sample.

## Project structure

```
main/
├── main.cpp                   # Entry point — init + run loop
├── AppFeed.hpp                # Audio source: exposes pre-recorded PCM clips one at a time
├── AppInference.hpp           # Runs ei_run_classifier and prints results
├── data/                      # Bundled WAV clips as C headers (gm_sample, unknown)
├── model-parameters/
│   ├── model_metadata.h       # EI classifier constants (labels, sample count, …)
│   └── model_variables.h      # Label strings and model config struct
└── tflite-model/
    ├── tflite_learn_*_compiled.cpp/.h  # Compiled TFLite model
    └── trained_model_ops_define.h      # Op resolver definitions
```

## Edge Impulse Project

https://studio.edgeimpulse.com/studio/972337/


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

# References

- https://docs.edgeimpulse.com/
- https://docs.edgeimpulse.com/knowledge/guides/getting-started-for-beginners
- https://docs.edgeimpulse.com/tutorials/end-to-end/keyword-spotting
- https://components.espressif.com/components/ozanoner/edgeimpulse-inference-sdk/versions/0.1.1/readme
- https://www.edgeimpulse.com/projects/all


