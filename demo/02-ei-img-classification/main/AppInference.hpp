// AppInference — runs the Edge Impulse image classifier on a single image buffer
// and logs memory usage, timing, and the top predicted label.

#pragma once

#include "edge-impulse-sdk/classifier/ei_print_results.h"
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "esp_log.h"
#include "model-parameters/model_metadata.h"

#include "AppInferenceBase.hpp"

namespace app
{
/// Drives one EI image-classification inference cycle.
/// Accepts a raw ARGB8888 image buffer, wraps it in an EI signal,
/// invokes `run_classifier`, and logs per-label scores and the top prediction.
class AppInference : public AppInferenceBase<uint32_t>
{
  public:
    AppInference() = default;
    ~AppInference() override = default;

    /// Initialize the inference engine, log model info and arena size, and reset current data
    /// pointer.
    void init() override
    {
#if CONFIG_EI_DISABLE_HW_ACCEL
        ESP_LOGW(TAG, "Hardware support is disabled for this build.");
#endif
        ESP_LOGI(TAG, "Model: %s", EI_CLASSIFIER_PROJECT_NAME);
        ESP_LOGI(TAG, "Labels: %d", EI_CLASSIFIER_LABEL_COUNT);

        current_data = nullptr;

        ESP_LOGI(TAG, "Model arena target: %u bytes",
                 static_cast<uint32_t>(EI_CLASSIFIER_TFLITE_LARGEST_ARENA_SIZE));
    }

    /// Stores @p data as the image source for the next `run()` call.
    /// Returns false if @p data is null or its length doesn't match the model's input frame size.
    bool feed(const raw_data_t<uint32_t>* const data) override
    {
        if (data == nullptr || data->length == 0)
        {
            return false;
        }
        current_data = data;
        return true;
    }

    /// EI signal callback: copies raw pixel values at [@p offset, @p offset+@p length)
    /// as floats into @p out_ptr. Returns -1 if out of bounds.
    int getSignalData(size_t offset, size_t length, float* out_ptr)
    {
        const size_t feature_count = EI_CLASSIFIER_RAW_SAMPLE_COUNT;
        if (offset + length > feature_count)
        {
            return -1;
        }

        for (size_t index = 0; index < length; index++)
        {
            out_ptr[index] = static_cast<float>(current_data->data[offset + index]);
        }
        return 0;
    }

    /// Builds the EI signal wrapper and calls `run_classifier`. Returns false on error.
    bool run() override
    {
        signal_t signal;
        signal.total_length = EI_CLASSIFIER_RAW_SAMPLE_COUNT;
        signal.get_data = [this](size_t offset, size_t length, float* out_ptr)
        { return this->getSignalData(offset, length, out_ptr); };

        EI_IMPULSE_ERROR err = run_classifier(&signal, &result, false);
        if (err != EI_IMPULSE_OK)
        {
            ESP_LOGE(TAG, "run_classifier failed (%d)", err);
            return false;
        }
        return true;
    }

    /// Logs per-label scores, the top prediction, and a PASS/FAIL check against the expected label.
    void handleResult() override
    {
        ESP_LOGI(TAG, "Predictions:");
        uint16_t top_index = 0;

        float top_value = result.classification[0].value;
        for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++)
        {
            if (result.classification[i].value > top_value)
            {
                top_value = result.classification[i].value;
                top_index = i;
            }
            ESP_LOGI(TAG, "  %s: %.5f", ei_classifier_inferencing_categories[i],
                     result.classification[i].value);
        }

        const char* predicted_label = ei_classifier_inferencing_categories[top_index];
        ESP_LOGI(TAG, "Top prediction: %s (%.5f)", predicted_label, top_value);
    }

  private:
    static constexpr const char* TAG = "inference";

    const raw_data_t<uint32_t>* current_data{nullptr};
    ei_impulse_result_t result{};
};
} // namespace app
