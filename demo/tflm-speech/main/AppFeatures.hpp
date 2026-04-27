
#pragma once

#include <algorithm>
#include "esp_log.h"

#include "tensorflow/lite/c/common.h"
#include "micro_features_generator.h"
#include "micro_model_settings.h"
#include "AppFeed.hpp"

// Binds itself to an area of memory intended to hold the input features for an
// audio-recognition neural network model, and fills that data area with the
// features representing the current audio input, for example from a microphone.
// The audio features themselves are a two-dimensional array, made up of
// horizontal slices representing the frequencies at one point in time, stacked
// on top of each other to form a spectrogram showing how those frequencies
// changed over time.

namespace app
{
    class AppFeatures
    {
    public:
        AppFeatures(int feature_size, int8_t *feature_data)
            : feature_size_(feature_size),
              feature_data_(feature_data),
              is_first_run_(true)
        {
            std::fill(feature_data_, feature_data_ + feature_size_, int8_t{0});
            if (InitializeMicroFeatures() != kTfLiteOk)
            {
                ESP_LOGE(TAG, "InitializeMicroFeatures failed");
            }
        }

        ~AppFeatures() = default;

        // Fills the feature data with information from audio inputs, and returns how
        // many feature slices were updated.
        TfLiteStatus PopulateFeatureData(int32_t last_time_in_ms, int32_t time_in_ms,
                                         int *how_many_new_slices, AppFeed *app_feed)
        {
            if (feature_size_ != kFeatureElementCount)
            {
                ESP_LOGE(TAG, "Requested feature_data_ size %d doesn't match %d",
                         feature_size_, kFeatureElementCount);
                return kTfLiteError;
            }

            // Quantize the time into steps as long as each window stride, so we can
            // figure out which audio data we need to fetch.
            const int last_step = (last_time_in_ms / kFeatureStrideMs);
            const int current_step = (time_in_ms / kFeatureStrideMs);

            int slices_needed = current_step - last_step;

            // If this is the first call, make sure we don't use any cached information.
            if (is_first_run_)
            {
                is_first_run_ = false;
                slices_needed = kFeatureCount;
            }

            if (slices_needed > kFeatureCount)
            {
                slices_needed = kFeatureCount;
            }
            *how_many_new_slices = slices_needed;

            const int slices_to_keep = kFeatureCount - slices_needed;
            const int slices_to_drop = kFeatureCount - slices_to_keep;
            // If we can avoid recalculating some slices, just move the existing data
            // up in the spectrogram, to perform something like this:
            // last time = 80ms          current time = 120ms
            // +-----------+             +-----------+
            // | data@20ms |         --> | data@60ms |
            // +-----------+       --    +-----------+
            // | data@40ms |     --  --> | data@80ms |
            // +-----------+   --  --    +-----------+
            // | data@60ms | --  --      |  <empty>  |
            // +-----------+   --        +-----------+
            // | data@80ms | --          |  <empty>  |
            // +-----------+             +-----------+
            if (slices_to_keep > 0)
            {
                for (int dest_slice = 0; dest_slice < slices_to_keep; ++dest_slice)
                {
                    int8_t *dest_slice_data =
                        feature_data_ + (dest_slice * kFeatureSize);
                    const int src_slice = dest_slice + slices_to_drop;
                    const int8_t *src_slice_data =
                        feature_data_ + (src_slice * kFeatureSize);
                    for (int i = 0; i < kFeatureSize; ++i)
                    {
                        dest_slice_data[i] = src_slice_data[i];
                    }
                }
            }

            if (slices_needed > 0)
            {
                for (int new_slice = slices_to_keep; new_slice < kFeatureCount;
                     ++new_slice)
                {
                    // const int new_step = (current_step - kFeatureCount + 1) + new_slice;
                    int16_t *audio_samples = nullptr;
                    int audio_samples_size = 0;

                    if (app_feed->GetAudioSamples(&audio_samples_size,
                                                  &audio_samples) != kTfLiteOk)
                    {
                        ESP_LOGW(TAG, "Failed to get audio samples for slice %d", new_slice);
                        return kTfLiteError;
                    }

                    if (audio_samples_size < kMaxAudioSampleSize)
                    {
                        ESP_LOGE(TAG, "Audio data size %d too small, want %d",
                                 audio_samples_size, kMaxAudioSampleSize);
                        return kTfLiteError;
                    }
                    int8_t *new_slice_data = feature_data_ + (new_slice * kFeatureSize);

                    TfLiteStatus generate_status = GenerateFeatures(
                        audio_samples, audio_samples_size, &g_features);
                    if (generate_status != kTfLiteOk)
                    {
                        return generate_status;
                    }

                    // copy features
                    for (int j = 0; j < kFeatureSize; ++j)
                    {
                        new_slice_data[j] = g_features[0][j];
                    }
                }
            }

            return kTfLiteOk;
        }

        void reset()
        {
            std::fill(feature_data_, feature_data_ + feature_size_, int8_t{0});
            is_first_run_ = true;
        }

    private:
        int feature_size_;
        int8_t *feature_data_;
        bool is_first_run_;

        Features g_features;
        const char *TAG = "feature_provider";
    };
}