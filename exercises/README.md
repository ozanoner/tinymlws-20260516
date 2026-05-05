# TinyML Workshop

Hands-on workshop to build, run, and iterate TinyML/Edge AI demos on ESP32-S3 using ESP-IDF and Wokwi.

## Prerequisites

Workshop details: https://www.eventbrite.co.uk/e/deploy-edgeai-applications-with-esp32-s3-a-hands-on-tinyml-workshop-tickets-1985133871849

- Install Docker, Git and VS Code on your host machine (as noted in the event page).
- The devcontainer already includes the required ESP-IDF toolchain and utilities.

### Using the ESP-IDF toolchain

From each project folder, use the following commands to build the examples:

```bash
# Wokwi-oriented build (default)
idf.py build

# Build (use target config from PRJ_TARGET)
PRJ_TARGET=esp32s3 idf.py build

# Flash + monitor on hardware
PRJ_TARGET=esp32s3 idf.py flash monitor -p /dev/ttyACM0
```

### Using Wokwi

- Edit `diagram.json` to add/adjust virtual peripherals (for example LEDs).
- Use Wokwi logs/serial output to validate model detections.


Client usage:

```bash
# Run simulation (requires wokwi.toml and diagram.json)
wokwi-cli 
```

Common parameters:
```
--timeout <ms> (default 30000ms): exits when timeout
--fail-text <text>: exits when fail-text encountered
--expect-text: exits when expect-text encountered
```


## Exercises

Update the example projects in [../demo](../demo) as described:

### 01 Edge Impulse keyword spotting exercise
**Goal**: KWS handling with an LED indicator.
1. Record and convert an audio file ("good morning") to a header. Integrate it in the app.
    ```bash
    $ wavtoh.sh good-morning.flac good-morning.flac.h
    ```
2. Add an LED in the Wokwi diagram. Signal it when the keyword is detected.

LED indicator: https://components.espressif.com/components/espressif/led_indicator/versions/2.1.2/readme?language=en
Example: https://github.com/espressif/esp-iot-solution/blob/master/examples/indicator/gpio/main/main.c


### 02 Edge Impulse image classification
**Goal**: Integrating an object detection model in an application.
1. Clone the project at https://studio.edgeimpulse.com/public/96468/latest
2. Download the C++ SDK and copy the model in the project
3. Test with a face pic
    ```bash
    $ imgtoh.sh face1.png face1.png.h
    ```
Object detection: https://docs.edgeimpulse.com/studio/projects/learning-blocks/blocks/object-detection/fomo



### 03 Tensorflow Lite Micro speech
**Goal**: Parameterized detection threshold in an TFLM KWS app.
1. Record and convert an audio file ("no") to a header. Integrate it in the app.
2. Modify `AppFeed.hpp` to feed "yes" first, then "no". 
3. Modify `AppInference.hpp`:
    - Parameterize the detection threshold in the `run` function
    - Break the main loop when "no" detected.

### 04 Tensorflow Lite Micro person detection 
1. Test with new images.
    ```bash
    $ imgtoh.sh face1.png face1.png.h -g
    ```
2. Use `tflite::AllOpsResolver` to add the ops and compare image sizes.
3. Optimize the arena size.
    ```c++
    tflite::MicroInterpreter::arena_used_bytes()
    ```

### 05 Espressif Wake-word detection
1. Test the app with the hilexin data (detection should fail).
2. Configure the app for hilexin and test.

### 06 Espressif face detection
Hardware only - no assignment

## What is next?
Try examples on real hardware.

Some devkits that you can use:
- ESP32-S3-EYE: https://github.com/espressif/esp-who/blob/master/docs/en/get-started/ESP32-S3-EYE_Getting_Started_Guide.md
- XIAO ESP32S3 Sense: https://www.seeedstudio.com/XIAO-ESP32S3-Sense-p-5639.html

## References

Espressif AIoT:
- https://docs.espressif.com/projects/esp-techpedia/en/latest/esp-friends/solution-introduction/ai/ai-solution.html

TFLM:
- https://ai.google.dev/edge/litert/microcontrollers/overview
- https://github.com/espressif/esp-tflite-micro

Edge Impulse:
- https://docs.edgeimpulse.com/tutorials/topics/inference/run-multiple-impulses-cpp
- https://docs.edgeimpulse.com/hardware/boards/seeed-xiao-esp32s3-sense
- https://components.espressif.com/components/ozanoner/edgeimpulse-inference-sdk/

