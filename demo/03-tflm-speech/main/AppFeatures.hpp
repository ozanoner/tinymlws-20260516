// AppFeatures - converts streaming PCM audio into the log-mel spectrogram
// slices expected by the TFLite Micro speech model.

#pragma once

#include "esp_log.h"
#include <algorithm>

#include "AppFeed.hpp"
#include "micro_features_generator.h"
#include "micro_model_settings.h"
#include "tensorflow/lite/c/common.h"

namespace app
{
class AppFeatures
{
  public:
    /// Constructs the feature provider, zeroes the spectrogram buffer, and
    /// initialises the underlying micro-features generator.
    AppFeatures(int feature_size, int8_t* feature_data)
        : feature_size(feature_size), feature_data(feature_data), is_first_run(true)
    {
        std::fill(feature_data, feature_data + feature_size, int8_t{0});
        if (InitializeMicroFeatures() != kTfLiteOk)
        {
            ESP_LOGE(TAG, "InitializeMicroFeatures failed");
        }
    }

    /// Default destructor; no dynamic resources are owned here.
    ~AppFeatures() = default;

    /// Slides the spectrogram forward in time: shifts existing slices up,
    /// then fills any new slots with log-mel features generated from the
    /// next audio window(s) pulled from `app_feed`. Sets `how_many_new_slices`
    /// to the number of slices that were actually computed.
    TfLiteStatus populateFeatureData(int32_t last_time_in_ms, int32_t time_in_ms,
                                     int* how_many_new_slices, AppFeed* app_feed)
    {
        if (feature_size != kFeatureElementCount)
        {
            ESP_LOGE(TAG, "Requested feature_data size %d doesn't match %d", feature_size,
                     kFeatureElementCount);
            return kTfLiteError;
        }

        // Convert millisecond timestamps to stride-aligned slice indices.
        const int last_step = (last_time_in_ms / kFeatureStrideMs);
        const int current_step = (time_in_ms / kFeatureStrideMs);

        // Number of new 20 ms slices that have elapsed since last call.
        int slices_needed = current_step - last_step;

        if (is_first_run)
        {
            is_first_run = false;
            slices_needed = kFeatureCount; // fill the entire spectrogram on the first call
        }

        if (slices_needed > kFeatureCount)
        {
            slices_needed = kFeatureCount; // can't compute more slices than the buffer holds
        }
        *how_many_new_slices = slices_needed;

        // Older slices that can be kept by shifting them toward index 0.
        const int slices_to_keep = kFeatureCount - slices_needed;
        // Slices at the front that are too old and will be overwritten.
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
                int8_t* dest_slice_data = feature_data + (dest_slice * kFeatureSize);
                const int src_slice = dest_slice + slices_to_drop;
                const int8_t* src_slice_data = feature_data + (src_slice * kFeatureSize);
                for (int i = 0; i < kFeatureSize; ++i)
                {
                    dest_slice_data[i] = src_slice_data[i];
                }
            }
        }

        if (slices_needed > 0)
        {
            for (int new_slice = slices_to_keep; new_slice < kFeatureCount; ++new_slice)
            {
                int16_t* audio_samples = nullptr;
                int audio_samples_size = 0;

                // Pull the next PCM window from the audio source.
                if (app_feed->getAudioSamples(&audio_samples_size, &audio_samples) != kTfLiteOk)
                {
                    ESP_LOGW(TAG, "Failed to get audio samples for slice %d", new_slice);
                    return kTfLiteError;
                }

                if (audio_samples_size < kMaxAudioSampleSize)
                {
                    ESP_LOGE(TAG, "Audio data size %d too small, want %d", audio_samples_size,
                             kMaxAudioSampleSize);
                    return kTfLiteError;
                }
                // Pointer to the row in the spectrogram that will receive the new slice.
                int8_t* new_slice_data = feature_data + (new_slice * kFeatureSize);

                // Compute log-mel for this PCM window.
                TfLiteStatus generate_status =
                    GenerateFeatures(audio_samples, audio_samples_size, &features);
                if (generate_status != kTfLiteOk)
                {
                    return generate_status;
                }

                // Copy the mel vector into the spectrogram row.
                for (int j = 0; j < kFeatureSize; ++j)
                {
                    new_slice_data[j] = features[0][j];
                }
            }
        }

        return kTfLiteOk;
    }

    /// Clears the spectrogram buffer and marks the next call to
    /// `populateFeatureData` as a first-run so all slices are recomputed.
    void reset()
    {
        std::fill(feature_data, feature_data + feature_size, int8_t{0});
        is_first_run = true;
    }

  private:
    Features features;
    int feature_size;
    int8_t* feature_data;

    bool is_first_run;

    const char* TAG = "features";
};
} // namespace app