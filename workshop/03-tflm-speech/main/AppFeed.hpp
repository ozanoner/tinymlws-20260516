// AppFeed - serves bundled yes/no PCM clips as overlapping audio windows for
// feature extraction and offline keyword-spotting inference.

#pragma once

#include "esp_log.h"
#include <algorithm>

#include "micro_model_settings.h"
#include "no_1000ms.wav.h"
#include "ringbuf.h"
#include "tensorflow/lite/c/common.h"
#include "yes_1000ms.wav.h"

#include "AppFeedBase.hpp"

constexpr int32_t history_samples_to_keep =
    ((kFeatureDurationMs - kFeatureStrideMs) * (kAudioSampleFrequency / 1000));

constexpr int32_t new_samples_to_get = (kFeatureStrideMs * (kAudioSampleFrequency / 1000));

namespace app
{
class AppFeed : public AppFeedBase<int16_t>
{
  public:
    virtual ~AppFeed() = default;

    /// Resets the history buffer to silence and clears the sample offset,
    /// preparing the window for the start of a new audio clip.
    void init() override
    {
        std::fill(std::begin(history_buffer), std::end(history_buffer), int16_t{0});
        offset_samples = 0;
        is_initialized = true;
    }

    /// Advances to the next bundled audio clip. Returns a pointer to its
    /// PCM descriptor, or nullptr when all clips have been consumed.
    const raw_data_t<int16_t>* next() override
    {
        ++current_index;

        if (current_index >= audio_data_cnt)
        {
            return nullptr;
        }

        is_initialized = false;
        return &audio_data[current_index];
    }

    /// Fills `audio_samples` with the next window (history + new
    /// samples) of the current clip with stride. Sets
    /// `audio_samples_size` to `kMaxAudioSampleSize` on success.
    TfLiteStatus getAudioSamples(int* audio_samples_size, int16_t** audio_samples)
    {
        // Descriptor for the clip currently selected by next().
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

        // Prepend the tail of the previous window to provide continuity across strides.
        std::copy(history_buffer, history_buffer + history_samples_to_keep, output_buffer);

        // Append the next 20 ms of new samples
        for (int i = 0; i < new_samples_to_get; ++i)
        {
            output_buffer[history_samples_to_keep + i] =
                prerecorded_pcm.data[(offset_samples + i) % kAudioSampleFrequency];
        }

        // Advance the read position by one stride, wrapping within the clip.
        offset_samples = (offset_samples + new_samples_to_get) % kAudioSampleFrequency;

        // Save the last 10 ms of this window as history for the next call.
        std::copy(output_buffer + new_samples_to_get,
                  output_buffer + new_samples_to_get + history_samples_to_keep, history_buffer);

        *audio_samples_size = kMaxAudioSampleSize;
        *audio_samples = output_buffer;
        return kTfLiteOk;
    }

    /// Returns the simulated audio timestamp in milliseconds, advancing
    /// by one feature stride (`kFeatureStrideMs`) on each call.
    int32_t latestAudioTimestamp()
    {
        timestamp_ms += kFeatureStrideMs;
        return timestamp_ms;
    }

  private:
    static constexpr const char* TAG = "feed";

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

} // namespace app
