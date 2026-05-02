
#pragma once

#include <algorithm>
#include "esp_log.h"

#include "tensorflow/lite/c/common.h"
#include "micro_features_generator.h"
#include "micro_model_settings.h"
#include "AppFeed.hpp"

namespace app
{
    class AppFeatures
    {
    public:
        AppFeatures(int feature_size, int8_t *feature_data)
            : feature_size(feature_size),
              feature_data(feature_data),
              is_first_run(true)
        {
            std::fill(feature_data, feature_data + feature_size, int8_t{0});
            if (InitializeMicroFeatures() != kTfLiteOk)
            {
                ESP_LOGE(TAG, "InitializeMicroFeatures failed");
            }
        }

        ~AppFeatures() = default;

        TfLiteStatus populateFeatureData(int32_t last_time_in_ms, int32_t time_in_ms,
                                         int *how_many_new_slices, AppFeed *app_feed)
        {
            if (feature_size != kFeatureElementCount)
            {
                ESP_LOGE(TAG, "Requested feature_data size %d doesn't match %d",
                         feature_size, kFeatureElementCount);
                return kTfLiteError;
            }

            const int last_step = (last_time_in_ms / kFeatureStrideMs);
            const int current_step = (time_in_ms / kFeatureStrideMs);

            int slices_needed = current_step - last_step;

            if (is_first_run)
            {
                is_first_run = false;
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
                        feature_data + (dest_slice * kFeatureSize);
                    const int src_slice = dest_slice + slices_to_drop;
                    const int8_t *src_slice_data =
                        feature_data + (src_slice * kFeatureSize);
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
                    int16_t *audio_samples = nullptr;
                    int audio_samples_size = 0;

                    if (app_feed->getAudioSamples(&audio_samples_size,
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
                    int8_t *new_slice_data = feature_data + (new_slice * kFeatureSize);

                    TfLiteStatus generate_status = GenerateFeatures(
                        audio_samples, audio_samples_size, &features);
                    if (generate_status != kTfLiteOk)
                    {
                        return generate_status;
                    }

                    for (int j = 0; j < kFeatureSize; ++j)
                    {
                        new_slice_data[j] = features[0][j];
                    }
                }
            }

            return kTfLiteOk;
        }

        void reset()
        {
            std::fill(feature_data, feature_data + feature_size, int8_t{0});
            is_first_run = true;
        }

    private:
        Features features;
        int feature_size;
        int8_t *feature_data;

        bool is_first_run;

        const char *TAG = "features";
    };
}