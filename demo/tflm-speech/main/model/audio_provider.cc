/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "audio_provider.h"

#include <cstdlib>
#include <cstring>

#include "freertos/FreeRTOS.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "freertos/task.h"
#include "ringbuf.h"
#include "micro_model_settings.h"

using namespace std;

static const char *TAG = "TF_LITE_AUDIO_PROVIDER";
/* ringbuffer to hold the incoming audio data */
ringbuf_t *g_audio_capture_buffer;
volatile int32_t g_latest_audio_timestamp = 0;
/* model requires 20ms new data from g_audio_capture_buffer and 10ms old data
 * each time , storing old data in the histrory buffer , {
 * history_samples_to_keep = 10 * 16 } */
constexpr int32_t history_samples_to_keep =
    ((kFeatureDurationMs - kFeatureStrideMs) *
     (kAudioSampleFrequency / 1000));
/* new samples to get each time from ringbuffer, { new_samples_to_get =  20 * 16
 * } */
constexpr int32_t new_samples_to_get =
    (kFeatureStrideMs * (kAudioSampleFrequency / 1000));

namespace
{
  int16_t g_audio_output_buffer[kMaxAudioSampleSize * 32];
  bool g_is_audio_initialized = false;
  int16_t g_history_buffer[history_samples_to_keep];

  extern const uint8_t no_1000ms_start[] asm("_binary_no_1000ms_wav_start");
  extern const uint8_t yes_1000ms_start[] asm("_binary_yes_1000ms_wav_start");

  constexpr int kWavHeaderBytes = 44;
  constexpr int kPrerecordedTotalSamples = 16000;
  constexpr int kPrerecordedStrideMs = kFeatureStrideMs;
  int32_t g_prerecorded_timestamp_ms = 0;
  int g_prerecorded_offset_samples = 0;

  const int16_t *GetPrerecordedPcmBase()
  {
    return reinterpret_cast<const int16_t *>(no_1000ms_start + kWavHeaderBytes);
  }

} // namespace

TfLiteStatus InitAudioRecording()
{
  memset(g_history_buffer, 0, sizeof(g_history_buffer));
  g_prerecorded_timestamp_ms = 0;
  g_prerecorded_offset_samples = 0;
  g_is_audio_initialized = true;
  ESP_LOGI(TAG, "Audio provider initialized in prerecorded mode");
  return kTfLiteOk;
}

TfLiteStatus GetAudioSamples(int start_ms, int duration_ms,
                             int *audio_samples_size, int16_t **audio_samples)
{
  (void)start_ms;
  (void)duration_ms;
  if (!g_is_audio_initialized)
  {
    TfLiteStatus init_status = InitAudioRecording();
    if (init_status != kTfLiteOk)
    {
      return init_status;
    }
    g_is_audio_initialized = true;
  }
  const int16_t *prerecorded_pcm = GetPrerecordedPcmBase();

  memcpy((void *)(g_audio_output_buffer), (void *)(g_history_buffer),
         history_samples_to_keep * sizeof(int16_t));

  for (int i = 0; i < new_samples_to_get; ++i)
  {
    g_audio_output_buffer[history_samples_to_keep + i] =
        prerecorded_pcm[(g_prerecorded_offset_samples + i) % kPrerecordedTotalSamples];
  }

  g_prerecorded_offset_samples =
      (g_prerecorded_offset_samples + new_samples_to_get) % kPrerecordedTotalSamples;

  memcpy((void *)(g_history_buffer),
         (void *)(g_audio_output_buffer + new_samples_to_get),
         history_samples_to_keep * sizeof(int16_t));

  *audio_samples_size = kMaxAudioSampleSize;
  *audio_samples = g_audio_output_buffer;
  return kTfLiteOk;
}

int32_t LatestAudioTimestamp()
{
  g_prerecorded_timestamp_ms += kPrerecordedStrideMs;
  return g_prerecorded_timestamp_ms;
}
