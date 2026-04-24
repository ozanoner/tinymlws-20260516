#pragma once

#include "esp_log.h"
#include "model-parameters/model_metadata.h"
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "edge-impulse-sdk/classifier/ei_print_results.h"
#include "edge-impulse-sdk/dsp/numpy.hpp"

#include "AppInferenceBase.hpp"

namespace app
{
    class AppInference : public AppInferenceBase<int16_t>
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
        }

        bool feed(const raw_data_t<int16_t> *const data) override
        {
            if (data == nullptr || data->length != EI_CLASSIFIER_RAW_SAMPLE_COUNT)
            {
                return false;
            }
            current_data = data;
            return true;
        }

        int getSignalData(size_t offset, size_t length, float *out_ptr)
        {
            // no data
            if (current_data == nullptr)
            {
                return -1;
            }
            // out of bounds
            if ((offset + length) > current_data->length)
            {
                return -1;
            }
            return ei::numpy::int16_to_float(&current_data->data[offset], out_ptr, length);
        }

        bool run() override
        {
            signal_t signal{};
            signal.total_length = EI_CLASSIFIER_RAW_SAMPLE_COUNT;
            signal.get_data = [this](size_t offset, size_t length, float *out_ptr)
            {
                return this->getSignalData(offset, length, out_ptr);
            };

            EI_IMPULSE_ERROR err = run_classifier(&signal, &result, false);
            if (err != EI_IMPULSE_OK)
            {
                ESP_LOGE(TAG, "run_classifier failed (%d)", err);
                return false;
            }
            return true;
        }

        void handleResult() override
        {
            ei_print_results(&ei_default_impulse, &result);
        }

    private:
        static constexpr const char *TAG = "inference";

        const raw_data_t<int16_t> *current_data{nullptr};
        ei_impulse_result_t result{};
    };
}
