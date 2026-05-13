# TinyML Workshop

Hands-on TinyML/Edge AI workshop for ESP32-S3 using ESP-IDF, Wokwi, Edge Impulse, TensorFlow Lite for Microcontrollers, and Espressif AIoT components.

## Workshop Objectives

- Understand the end-to-end TinyML pipeline from model selection to embedded deployment.
- Run and compare speech and vision examples across different frameworks.
- Practice working with memory/runtime constraints on embedded targets.

## Prerequisites

### Hardware

- Base workshop flow can run with Wokwi (no physical board required).
- You can also use a physical ESP32-S3 board if you have one.

### Software

- Visual Studio Code (latest) with Microsoft Remote Development extension pack.
- Docker (latest).
- Git (latest).
- Wokwi account + CI token.

Detailed environment setup: [docs/setup.md](./docs/setup.md)

## Quick Start

1. Clone this repository.
2. Open it in VS Code.
3. Reopen in Dev Container.
4. Create `.env` in the workspace root with `WOKWI_CLI_TOKEN=...`.
5. Pick a demo project and build it from that demo folder.

Example build commands from inside a demo folder:

```bash
# Wokwi-oriented build
idf.py build

# Hardware build and run on ESP32-S3
PRJ_TARGET=esp32s3 idf.py flash monitor -p /dev/ttyACM0
```

## Repository Layout

- `.devcontainer/`: Dev container configuration and toolchain bootstrap.
- `demo/`: Reference TinyML demos.
- `exercises/`: Workshop exercises and participant tasks.
- `exercises/solutions/`: Reference solution snapshots.
- `docs/`: Setup and workshop documentation.
- `tools/`: Utility scripts (`wavtoh.sh`, `imgtoh.sh`, `decode_backtrace.sh`).

## Demo Tracks

| Demo | Folder | Framework | Task |
|---|---|---|---|
| 01 | `demo/01-ei-keyword-spotting` | Edge Impulse | Keyword spotting (`good_morning` vs `unknown`) |
| 02 | `demo/02-ei-img-classification` | Edge Impulse | Image classification |
| 03 | `demo/03-tflm-speech` | TFLite Micro | Speech keyword spotting (`yes/no/silence/unknown`) |
| 04 | `demo/04-tflm-person-detection` | TFLite Micro | Person detection (`person` / `no person`) |
| 05 | `demo/05-esp-wake-word` | ESP-SR | Wake-word detection |
| 06 | `demo/06-esp-face-detection` | ESP-WHO / esp-dl | Face detection |

Each demo includes its own README with build/run notes and model-specific details.

## Exercises

- Exercises are documented in [exercises/README.md](./exercises/README.md).
- Reference implementations are under `exercises/solutions/`.

## Common Utilities

- `tools/wavtoh.sh`: Convert WAV audio into C/C++ headers for bundled test samples.
- `tools/imgtoh.sh`: Convert and resize images into C/C++ headers (supports grayscale mode).
- `tools/decode_backtrace.sh`: Decode ESP-IDF backtraces.

## Agenda

| Section | Duration | Topics |
|---|---:|---|
| Introduction | 15 min | Objectives, roadmap, environment overview |
| Technology Review | 15 min | TinyML concepts, ESP-IDF flow, framework comparison |
| Edge Impulse | ~1 hr | Keyword spotting and image classification |
| TensorFlow Lite Micro | ~1 hr | Speech and person detection demos |
| Espressif AIoT | ~1 hr | Wake-word and face detection demos |
| Recap and Q&A | 15 min | Summary and next steps |

## References

### Edge Impulse

- https://docs.edgeimpulse.com/tutorials/end-to-end/keyword-spotting
- https://docs.edgeimpulse.com/tutorials/end-to-end/image-classification

### TFLite Micro

- https://github.com/espressif/esp-tflite-micro/tree/master/examples/micro_speech
- https://github.com/espressif/esp-tflite-micro/tree/master/examples/person_detection

### Espressif AIoT

- ESP-SR examples: https://github.com/espressif/esp-skainet/tree/master/examples
- ESP-WHO examples: https://github.com/espressif/esp-who/tree/master/examples
- The deep-learning framework (ESP-DL): https://github.com/espressif/esp-dl
- The neural-network library (ESP-NN): https://github.com/espressif/esp-nn
- The speech recognition framework (ESP-SR): https://github.com/espressif/esp-sr

## Suggested Boards

- ESP32-S3-EYE: https://github.com/espressif/esp-who/blob/master/docs/en/get-started/ESP32-S3-EYE_Getting_Started_Guide.md
- XIAO ESP32S3 Sense: https://www.seeedstudio.com/XIAO-ESP32S3-Sense-p-5639.html

## Troubleshooting

- If Dev Container build fails, verify Docker is running and `.env` exists at repository root.
- Ensure `WOKWI_CLI_TOKEN` is set before running Wokwi CLI.
- If a demo resets or stalls, review watchdog settings and serial logs for backtrace output.