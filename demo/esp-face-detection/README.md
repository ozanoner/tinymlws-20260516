# ESP face detection example

Offline face-detection demo for ESP32-S3 using Espressif's `human_face_detect` component.
The app iterates over five bundled `160x120` RGB888 frames and reports detected faces with their label ID and score.

`AppFeed` exposes the pre-recorded frames from `data/offline_frames.hpp` one at a time. For each frame, `AppInference` builds a `dl::image::img_t`, runs `HumanFaceDetect::run(...)`, and logs either `No detection` or one line per detected face.

## Project structure

```
main/
├── main.cpp              # Entry point — init + offline inference loop
├── AppFeed.hpp           # Image source: exposes bundled RGB frames one at a time
└── AppInference.hpp      # Runs HumanFaceDetect and logs detections

data/
└── offline_frames.hpp    # Five bundled 160x120 RGB888 test frames

managed_components/
├── espressif__human_face_detect/  # Face-detection model/component
├── espressif__esp-dl/             # ESP deep-learning runtime
├── espressif__esp-dsp/            # DSP support library
├── espressif__esp_new_jpeg/       # Image helper dependency
└── espressif__dl_fft/             # FFT utility dependency
```

## Running on ESP32-S3

Build:

```bash
idf.py build
```

Run:

```bash
idf.py flash monitor -p /dev/ttyACM0
```

# References

- https://components.espressif.com/components/espressif/human_face_detect
- https://github.com/espressif/esp-dl
- https://github.com/espressif/esp-dl/tree/master/docs/en/tutorials
- https://github.com/espressif/esp-dl/blob/master/docs/en/tutorials/how_to_run_model.rst
- https://github.com/espressif/esp-dl/blob/master/esp-dl/vision/detect/dl_detect_define.hpp
- https://github.com/ultralytics/ultralytics
- https://roboflow.com/


