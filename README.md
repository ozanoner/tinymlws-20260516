# TinyML Workshop

This repository is the scaffold for a hands-on TinyML workshop focused on Espressif hardware, TensorFlow Lite for Microcontrollers, ESP AIoT frameworks, and Edge Impulse workflows.

## Workshop Objectives

- Understand the TinyML pipeline from model selection to edge deployment.
- Run and compare reference examples across speech, vision, and keyword/image classification tasks.
- Review practical constraints on embedded targets (memory, watchdogs, acceleration support).

## Prerequisites

### Hardware

- No physical hardware is required for the base workshop flow.
- Wokwi is used as the emulator for the examples.

### Software

- Ubuntu 24.04
- Visual Studio Code (latest)
- Docker (latest)
- Git (latest)
- Wokwi Community License

## Repository Layout

- `.devcontainer/`: Development container definition (ESP-IDF toolchain and workspace environment).
- `extra_components/`: ESP-IDF components used by workshop examples.

## Quick Start

1. Clone this repository.
2. Open the folder in VS Code.
3. Reopen in container when prompted (or use `Dev Containers: Reopen in Container`).
4. Verify Docker is running and the container build completes.
5. Sign in to Wokwi and ensure your community license is active.

## Agenda

| Section | Duration | Topics |
|---|---:|---|
| Introduction | 15 min | Objectives, roadmap, Wokwi sign-up, scaffold + dev container cloning |
| Technology Review | 15 min | TinyML concepts and pipeline, Espressif AIoT stack, ESP-IDF toolchain |
| TensorFlow Lite for Microcontrollers | ~1 hr | Intro, speech example, person detection example |
| ESP32 AIoT | ~1 hr | Intro/frameworks, ESP-SR wake word + commands, ESP-WHO detection |
| Edge Impulse | ~1 hr | Platform intro, keyword spotting, image classification |
| Recap | 10 min | Summary and key takeaways |
| Q&A | 10 min | Open discussion |

## Example Tracks And References

### TensorFlow Lite for Microcontrollers

- Micro speech example: https://github.com/espressif/esp-tflite-micro/tree/master/examples/micro_speech
- Person detection example: https://github.com/espressif/esp-tflite-micro/tree/master/examples/person_detection

### ESP AIoT (ESP-SR / ESP-WHO)

- ESP-SR examples: https://github.com/espressif/esp-skainet/tree/master/examples
- ESP-WHO object detect: https://github.com/espressif/esp-who/tree/master/examples/object_detect

### Edge Impulse

- Keyword spotting tutorial: https://docs.edgeimpulse.com/tutorials/end-to-end/keyword-spotting
- Image classification tutorial: https://docs.edgeimpulse.com/tutorials/end-to-end/image-classification

## Suggested Boards

- ESP32-S3-EYE: https://github.com/espressif/esp-who/blob/master/docs/en/get-started/ESP32-S3-EYE_Getting_Started_Guide.md
- Alternative board reference: https://www.seeedstudio.com/XIAO-ESP32S3-Sense-p-5639.html

## Known Notes

- If examples stall/reset, verify watchdog settings (disable WDT or increase threshold for debugging).
- Some flows may run without ESP-NN or dedicated hardware acceleration; expect lower performance.


## Status

This is an initial README version generated from the current workshop outline. It will be expanded with concrete example commands, file paths, and troubleshooting playbooks as implementation content is added to the repository.
