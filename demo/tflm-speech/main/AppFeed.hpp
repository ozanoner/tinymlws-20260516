
#pragma once

#include <algorithm>
#include "esp_log.h"

#include "tensorflow/lite/c/common.h"
#include "micro_model_settings.h"
#include "ringbuf.h"
#include "yes_1000ms.wav.h"
#include "no_1000ms.wav.h"

#include "AppFeedBase.hpp"

constexpr int32_t history_samples_to_keep =
    ((kFeatureDurationMs - kFeatureStrideMs) *
     (kAudioSampleFrequency / 1000));

constexpr int32_t new_samples_to_get =
    (kFeatureStrideMs * (kAudioSampleFrequency / 1000));

namespace app
{
    class AppFeed : public AppFeedBase<int16_t>
    {
    public:
        virtual ~AppFeed() = default;

        void init() override
        {
            memset(history_buffer, 0, sizeof(history_buffer));
            offset_samples = 0;
            is_initialized = true;
        }

        const raw_data_t<int16_t> *next() override
        {
            ++current_index;

            if (current_index >= audio_data_cnt)
            {
                return nullptr;
            }

            is_initialized = false;
            return &audio_data[current_index];
        }

        TfLiteStatus getAudioSamples(int *audio_samples_size, int16_t **audio_samples)
        {
            auto prerecorded_pcm = audio_data[current_index];

            if (prerecorded_pcm.data == nullptr || prerecorded_pcm.length == 0)
            {
                ESP_LOGE(TAG, "Invalid audio data");
                return kTfLiteError;
            }

            if (!is_initialized)
            {
                init();
            }

            memcpy(output_buffer, history_buffer,
                   history_samples_to_keep * sizeof(int16_t));

            for (int i = 0; i < new_samples_to_get; ++i)
            {
                output_buffer[history_samples_to_keep + i] =
                    prerecorded_pcm.data[(offset_samples + i) % kAudioSampleFrequency];
            }

            offset_samples =
                (offset_samples + new_samples_to_get) % kAudioSampleFrequency;

            memcpy(history_buffer,
                   output_buffer + new_samples_to_get,
                   history_samples_to_keep * sizeof(int16_t));

            *audio_samples_size = kMaxAudioSampleSize;
            *audio_samples = output_buffer;
            return kTfLiteOk;
        }

        int32_t latestAudioTimestamp()
        {
            timestamp_ms += kFeatureStrideMs;
            return timestamp_ms;
        }

    private:
        static constexpr const char *TAG = "feed";

        bool is_initialized = false;
        int16_t output_buffer[kMaxAudioSampleSize * 32];
        int16_t history_buffer[history_samples_to_keep];

        int32_t timestamp_ms = 0;
        int offset_samples = 0;

        static constexpr raw_data_t<int16_t> audio_data[] = {
            {kOfflineKeywordSample_no, kOfflineKeywordSampleLength_no},
            {kOfflineKeywordSample_yes, kOfflineKeywordSampleLength_yes}};
        static const size_t audio_data_cnt = sizeof(audio_data) / sizeof(audio_data[0]);

        int current_index = -1;
    };

}
