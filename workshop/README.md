# TinyML Workshop

## Prerequisites

### Using the ESP-IDF toolchain

### Using Wokwi

## IDF components
- https://components.espressif.com/components/espressif/esp-sr/versions/2.4.3/readme
- https://components.espressif.com/components/ozanoner/edgeimpulse-inference-sdk/versions/0.1.1/readme


## Assignments

### 01 Edge Impulse keyword spotting
1. Record and convert an audio file ("good morning") to a header. Integrate it in the app.
    ```bash
    $ wavtoh.sh good-morning.flac good-morning.flac.h
    ```
2. Add an LED in the Wokwi diagram. Signal it when the keyword detected.


### 02 Edge Impulse image classification
1. Clone the project at https://studio.edgeimpulse.com/public/96468/latest
2. Download the C++ SDK and copy the model in the project
3. Test with a face pic
    ```bash
    $ imgtoh.sh face1.png face1.png.h
    ```

### 03 Tensorflow Lite Micro speech
1. Record and convert an audio file ("yes") to a header. Integrate in the app.
2. Modify `AppFeed.hpp` to return the "yes" data every time (no nullptr return).
3. Modify `AppInference.hpp`:
    - Parameterize the detection threshold in the `run` function
    - Loop in the main app to find the detection point. Break when found.

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
1. Test the app with the hilexin data (detection should fail)
2. Configure the app for hilexin and test

### Espressif face detection
Hardware only.

## What is next?
- Try examples on real hardware

## References

TFLM:
- https://github.com/tensorflow/tflite-micro/tree/main/tensorflow/lite/micro/examples/memory_footprint
- 