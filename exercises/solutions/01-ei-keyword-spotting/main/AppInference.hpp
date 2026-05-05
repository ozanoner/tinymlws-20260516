// AppInference — runs the Edge Impulse classifier on a PCM clip and prints results.

#pragma once

#include "edge-impulse-sdk/classifier/ei_print_results.h"
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "edge-impulse-sdk/dsp/numpy.hpp"
#include "esp_log.h"
#include "model-parameters/model_metadata.h"

#include "AppIndicator.hpp"
#include "AppInferenceBase.hpp"

namespace app
{
/// Drives one EI inference cycle: accepts a raw PCM buffer, converts it to a
/// float signal, invokes `run_classifier`, and prints label scores.
class AppInference : public AppInferenceBase<int16_t>
{
  public:
    AppInference() = default;
    ~AppInference() override = default;

    /// Logs model name and label count; resets current data pointer.
    void init() override
    {
#if CONFIG_EI_DISABLE_HW_ACCEL
        ESP_LOGW(TAG, "Hardware support is disabled for this build.");
#endif
        ESP_LOGI(TAG, "Model: %s", EI_CLASSIFIER_PROJECT_NAME);
        ESP_LOGI(TAG, "Labels: %d", EI_CLASSIFIER_LABEL_COUNT);

        current_data = nullptr;
        indicator.init();
    }

    /// Stores @p data as the source for the next `run()` call.
    bool feed(const raw_data_t<int16_t>* const data) override
    {
        if (data == nullptr || data->length == 0)
        {
            return false;
        }
        current_data = data;
        return true;
    }

    /// EI signal callback: converts int16 samples at [@p offset, @p offset+@p length)
    /// to float into @p out_ptr.
    int getSignalData(size_t offset, size_t length, float* out_ptr)
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

    /// Builds the EI signal wrapper and calls `run_classifier`.
    bool run() override
    {
        // Wrap the stored PCM buffer in an EI signal_t so the SDK can
        // pull float samples on demand via the get_data callback.
        signal_t signal{};
        signal.total_length = EI_CLASSIFIER_RAW_SAMPLE_COUNT;
        signal.get_data = [this](size_t offset, size_t length, float* out_ptr)
        { return this->getSignalData(offset, length, out_ptr); };

        // Run DSP + inference pipeline; result holds per-label scores.
        EI_IMPULSE_ERROR err = run_classifier(&signal, &result, false);
        if (err != EI_IMPULSE_OK)
        {
            ESP_LOGE(TAG, "run_classifier failed (%d)", err);
            return false;
        }
        return true;
    }

    /// Prints classifier results via `ei_print_results`.
    void handleResult() override
    {
        ei_print_results(&ei_default_impulse, &result);

        if (result.classification[0].value > .3)
        {
            indicator.blink();
        }
        else
        {
            ESP_LOGI(TAG, "Keyword not detected");
        }
    }

  private:
    static constexpr const char* TAG = "inference";

    const raw_data_t<int16_t>* current_data{nullptr};
    ei_impulse_result_t result{};
    AppIndicator indicator{};
};
} // namespace app
