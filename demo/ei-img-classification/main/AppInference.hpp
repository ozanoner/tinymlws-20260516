#pragma once

#include "esp_log.h"
#include "esp_heap_caps.h"
#include "model-parameters/model_metadata.h"
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "edge-impulse-sdk/classifier/ei_print_results.h"
#include "edge-impulse-sdk/dsp/numpy.hpp"

#include "AppInferenceBase.hpp"

namespace app
{
    class AppInference : public AppInferenceBase<uint32_t>
    {
    public:
        AppInference() = default;
        ~AppInference() override = default;

        void init() override
        {
#if CONFIG_EI_DISABLE_HW_ACCEL
            ESP_LOGW(TAG, "Hardware support is disabled for this build.");
#endif
            ESP_LOGI(TAG, "Model: %s", EI_CLASSIFIER_PROJECT_NAME);
            ESP_LOGI(TAG, "Labels: %d", EI_CLASSIFIER_LABEL_COUNT);

            current_data = nullptr;

            size_t internal_free = heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
            size_t internal_largest =
                heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);

            ESP_LOGI(TAG, "Heap (internal): free=%u, largest=%u bytes",
                     static_cast<unsigned int>(internal_free), static_cast<unsigned int>(internal_largest));

            size_t psram_free = heap_caps_get_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
            size_t psram_largest = heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
            ESP_LOGI(TAG, "PSRAM: enabled, free=%u, largest=%u bytes",
                     static_cast<unsigned int>(psram_free), static_cast<unsigned int>(psram_largest));

            ESP_LOGI(TAG, "Model arena target: %u bytes",
                     static_cast<unsigned int>(EI_CLASSIFIER_TFLITE_LARGEST_ARENA_SIZE));
        }

        bool feed(const raw_data_t<uint32_t> *const data) override
        {
            if (data == nullptr || data->length != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE)
            {
                return false;
            }
            current_data = data;
            return true;
        }

        int getSignalData(size_t offset, size_t length, float *out_ptr)
        {
            const size_t feature_count = sizeof(features) / sizeof(features[0]);
            if (offset + length > feature_count)
            {
                return -1;
            }

            for (size_t index = 0; index < length; index++)
            {
                out_ptr[index] = static_cast<float>(features[offset + index]);
            }
            return 0;
        }

        bool run() override
        {
            signal_t features_signal;
            features_signal.total_length = sizeof(features) / sizeof(features[0]);
            // features_signal.get_data = &raw_feature_get_data;
            features_signal.get_data = [this](size_t offset, size_t length, float *out_ptr)
            {
                return this->getSignalData(offset, length, out_ptr);
            };

            EI_IMPULSE_ERROR err = run_classifier(&features_signal, &result, false);
            if (err != EI_IMPULSE_OK)
            {
                ESP_LOGE(TAG, "run_classifier failed (%d)", err);
                return false;
            }
            return true;
        }

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

            const char *expected_label = "plant";
            const char *predicted_label = ei_classifier_inferencing_categories[top_index];
            ESP_LOGI(TAG, "Top prediction: %s (%.5f)", predicted_label, top_value);
            ESP_LOGI(TAG, "Expected label: %s -> %s", expected_label,
                     strcmp(predicted_label, expected_label) == 0 ? "PASS" : "FAIL");
        }

    private:
        static constexpr const char *TAG = "inference";

        const raw_data_t<uint32_t> *current_data{nullptr};
        ei_impulse_result_t result{};
    };
}
