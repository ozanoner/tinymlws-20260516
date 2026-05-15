// AppInference — loads the WakeNet wn9s_hiesp model from the 'model' partition
// and detects the "Hi ESP" wake word in each incoming PCM chunk.

#pragma once

#include "esp_wn_iface.h"
#include "esp_wn_models.h"
#include "model_path.h"
#include <cstdlib>

#include "esp_log.h"

#include "AppInferenceBase.hpp"

namespace app
{
/// Wraps the ESP-SR WakeNet model lifecycle and performs wake-word detection
/// on successive PCM audio chunks provided by AppFeed.
class AppInference : public AppInferenceBase<int16_t>
{
  public:
    AppInference() = default;

    /// Releases the WakeNet model instance if it was created.
    ~AppInference() override
    {
        if (wakenet != nullptr && model_instance != nullptr)
        {
            wakenet->destroy(model_instance);
        }
    }

    /// Loads the WakeNet model from the `model` partition and caches the
    /// audio chunk size expected by the detector.
    void init() override
    {
        // access to the models in the 'model' partition
        models = esp_srmodel_init("model");
        if (models == nullptr)
        {
            ESP_LOGE(TAG, "Failed to init models from partition 'model'");
            return;
        }

        // find the model name for the "Hi ESP" wake word
        char* model_name = esp_srmodel_filter(models, ESP_WN_PREFIX, "hiesp");
        if (model_name == nullptr)
        {
            ESP_LOGE(TAG, "No wake model found for hiesp");
            return;
        }
        else
        {
            ESP_LOGI(TAG, "Found model: %s", model_name);
        }

        // get the wakenet interface for the model
        wakenet = (esp_wn_iface_t*)esp_wn_handle_from_name(model_name);
        if (wakenet == nullptr)
        {
            ESP_LOGE(TAG, "Failed to get wakenet handle for model: %s", model_name);
            return;
        }

        // initialize the model instance with the default detection mode (DET_MODE_95)
        // aggressive detection. more false positives.
        // load model data and get ready for detection
        model_instance = wakenet->create(model_name, DET_MODE_95);
        if (model_instance == nullptr)
        {
            ESP_LOGE(TAG, "Failed to create model: %s", model_name);
            return;
        }

        audio_chunksize = wakenet->get_samp_chunksize(model_instance) * sizeof(int16_t);
    }

    /// Returns the detector input chunk size in bytes.
    int getAudioChunkSize()
    {
        return audio_chunksize;
    }

    /// Stores the next PCM chunk to be processed.
    bool feed(const raw_data_t<int16_t>* const data) override
    {
        if (data == nullptr || data->length == 0)
        {
            return false;
        }
        current_data = data;
        return true;
    }

    /// Runs WakeNet on the current PCM chunk and records whether the wake
    /// word was detected.
    bool run() override
    {
        wakenet_state_t state =
            wakenet->detect(model_instance, const_cast<int16_t*>(current_data->data));
        if (state == WAKENET_DETECTED)
        {
            detected = true;
        }
        return true;
    }

    /// Logs the detection result and clears the one-shot detected flag.
    void handleResult() override
    {
        if (detected)
        {
            ESP_LOGI(TAG, "Wake word detected!");
            detected = false;
        }
    }

  private:
    static constexpr const char* TAG = "inference";

    srmodel_list_t* models{nullptr};
    esp_wn_iface_t* wakenet{nullptr};
    model_iface_data_t* model_instance{nullptr};
    int audio_chunksize{0};
    const raw_data_t<int16_t>* current_data{nullptr};

    bool detected{false};
};
} // namespace app
