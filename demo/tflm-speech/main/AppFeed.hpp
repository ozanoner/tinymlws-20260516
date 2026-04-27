
#pragma once

#include "tensorflow/lite/c/common.h"
#include "micro_model_settings.h"
#include "ringbuf.h"

#include "esp_log.h"

#include "AppFeedBase.hpp"

constexpr int32_t history_samples_to_keep =
    ((kFeatureDurationMs - kFeatureStrideMs) *
     (kAudioSampleFrequency / 1000));
/* new samples to get each time from ringbuffer, { new_samples_to_get =  20 * 16
 * } */
constexpr int32_t new_samples_to_get =
    (kFeatureStrideMs * (kAudioSampleFrequency / 1000));

extern const uint8_t no_1000ms_start[] asm("_binary_no_1000ms_wav_start");
extern const uint8_t no_1000ms_end[] asm("_binary_no_1000ms_wav_end");
extern const uint8_t yes_1000ms_start[] asm("_binary_yes_1000ms_wav_start");
extern const uint8_t yes_1000ms_end[] asm("_binary_yes_1000ms_wav_end");

namespace app
{
    class AppFeed : public AppFeedBase<int16_t>
    {
    public:
        virtual ~AppFeed() = default;

        void init() override
        {
            memset(g_history_buffer, 0, sizeof(g_history_buffer));
            g_prerecorded_offset_samples = 0;
            g_is_audio_initialized = true;
        }

        const raw_data_t<int16_t> *next() override
        {
            ++current_index;

            if (current_index >= data_len)
            {
                ESP_LOGW(TAG, "No more data to feed");
                return nullptr;
            }

            g_is_audio_initialized = false;
            return &getData()[current_index];
        }

        TfLiteStatus GetAudioSamples(int *audio_samples_size, int16_t **audio_samples)
        {
            auto prerecorded_pcm = getData()[current_index];

            if (prerecorded_pcm.data == nullptr || prerecorded_pcm.length == 0)
            {
                ESP_LOGE(TAG, "Invalid audio data");
                return kTfLiteError;
            }

            if (!g_is_audio_initialized)
            {
                init();
            }

            memcpy((void *)(g_audio_output_buffer), (void *)(g_history_buffer),
                   history_samples_to_keep * sizeof(int16_t));

            for (int i = 0; i < new_samples_to_get; ++i)
            {
                g_audio_output_buffer[history_samples_to_keep + i] =
                    prerecorded_pcm.data[(g_prerecorded_offset_samples + i) % kPrerecordedTotalSamples];
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

    private:
        static constexpr const char *TAG = "feed";

        // From model
        int16_t g_audio_output_buffer[kMaxAudioSampleSize * 32];
        bool g_is_audio_initialized = false;
        int16_t g_history_buffer[history_samples_to_keep];

        static const int kWavHeaderBytes = 44;
        static const int kPrerecordedTotalSamples = 16000;
        static const int kPrerecordedStrideMs = kFeatureStrideMs;
        int32_t g_prerecorded_timestamp_ms = 0;
        int g_prerecorded_offset_samples = 0;

        // class internals
        static const raw_data_t<int16_t> *getData()
        {
            static const raw_data_t<int16_t> data[] = {
                {reinterpret_cast<const int16_t *>(no_1000ms_start + kWavHeaderBytes),
                 (no_1000ms_end - no_1000ms_start - kWavHeaderBytes) / sizeof(int16_t)},
                {reinterpret_cast<const int16_t *>(yes_1000ms_start + kWavHeaderBytes),
                 (yes_1000ms_end - yes_1000ms_start - kWavHeaderBytes) / sizeof(int16_t)}};
            return data;
        }

        static constexpr size_t data_len = 2;

        int current_index = -1;
    };

}
