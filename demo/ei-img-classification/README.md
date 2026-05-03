# Edge Impulse image classification example

Offline image classification example.

The app loads a bundled test image, runs a single inference, logs memory usage and timing information, and reports the top prediction.

## Notes

- Impulse: https://studio.edgeimpulse.com/studio/904830/impulse/1/deployment
- Use [tools/imgtoh.sh](../../tools/imgtoh.sh) to resize an input image to 96x96 and convert it into a C header for bundled test data.

## Project structure

```
main/
├── main.cpp                   # Entry point — init + single inference loop
├── AppFeed.hpp                # Image source: exposes bundled test image(s) one at a time
├── AppInference.hpp           # Runs ei_run_classifier and reports top prediction
├── data/
│   └── offline_sample.h       # Test image as a C array (use tools/imgtoh.sh to regenerate)
├── model-parameters/
│   ├── model_metadata.h       # EI classifier constants (input size, label count, …)
│   └── model_variables.h      # Label strings and model config struct
└── tflite-model/
    ├── tflite_learn_*_compiled.cpp/.h  # Compiled TFLite model
    └── trained_model_ops_define.h      # Op resolver definitions
```

## Build and run

Note: Default build (ie. when no `PRJ_TARGET` is provided) is Wokwi.

### Wokwi

Build:

```bash
rm -rf sdkconfig build/ && PRJ_TARGET=wokwi idf.py build
```

Run:

```bash
wokwi-cli . --timeout 150000 --fail-text "Backtrace:" --expect-text "main_task: Returned from app_main()"
```

### ESP32-S3 hardware

Build:

```bash
rm -rf sdkconfig build/ && PRJ_TARGET=esp32s3 idf.py build
```

Run:

```bash
idf.py flash monitor
```

## References
- https://docs.edgeimpulse.com/
- https://docs.edgeimpulse.com/knowledge/guides/getting-started-for-beginners
- https://docs.edgeimpulse.com/tutorials/end-to-end/keyword-spotting
- https://components.espressif.com/components/ozanoner/edgeimpulse-inference-sdk/versions/0.1.1/readme
- https://www.edgeimpulse.com/projects/all


