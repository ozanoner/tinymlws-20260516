// AppInference - runs the TFLite Micro speech model on generated features and
// logs keyword detections for the offline audio clips.

#pragma once

#include "esp_log.h"
#include <algorithm>
#include <cstdint>
#include <iterator>

#include "micro_model_settings.h"
#include "model.h"
#include "sdkconfig.h"
#include "tensorflow/lite/core/c/common.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "AppFeatures.hpp"
#include "AppFeed.hpp"
#include "AppInferenceBase.hpp"

#define TENSOR_ARENA_SIZE (30 * 1024)

namespace app
{
class AppInference : public AppInferenceBase<int16_t>
{
  public:
    /// Constructs the offline speech inference pipeline.
    AppInference() = default;

    /// Cleans up the inference object; no dynamic resources are owned here.
    ~AppInference() override = default;

    /// Initializes the TFLite Micro model, interpreter, input tensor, and
    /// feature provider used for keyword spotting.
    void init() override
    {
        // Access the compiled flatbuffer model stored in flash.
        model = tflite::GetModel(g_model);

        // An easier approach is to just use the AllOpsResolver
        // tflite::AllOpsResolver resolver;
        static tflite::MicroMutableOpResolver<4> micro_op_resolver;
        micro_op_resolver.AddDepthwiseConv2D();
        micro_op_resolver.AddFullyConnected();
        micro_op_resolver.AddSoftmax();
        micro_op_resolver.AddReshape();

        // Build an interpreter to run the model with.
        static tflite::MicroInterpreter static_interpreter(model, micro_op_resolver, tensor_arena,
                                                           TENSOR_ARENA_SIZE);
        interpreter = &static_interpreter;

        // Allocate memory from the tensor_arena for the model's tensors.
        interpreter->AllocateTensors();

        // Get information about the memory area to use for the model's input.
        model_input = interpreter->input(0);
        model_input_buffer = tflite::GetTensorData<int8_t>(model_input);

        // Reuse a single feature provider backed by the persistent feature buffer.
        static AppFeatures static_feature_provider(kFeatureElementCount, feature_buffer);
        feature_provider = &static_feature_provider;

        previous_time = 0;
    }

    /// Advances to the next audio slice, updates the spectrogram features,
    /// and invokes the model when enough new data is available.
    bool run() override
    {
        auto next_data = app_feed.next();
        if (next_data == nullptr || next_data->data == nullptr || next_data->length == 0)
        {
            ESP_LOGW(TAG, "No more data to feed");
            return false;
        }

        const int32_t current_time = app_feed.latestAudioTimestamp();

        int how_many_new_slices = 0;
        // Convert the latest audio window(s) into the rolling spectrogram input.
        TfLiteStatus feature_status = feature_provider->populateFeatureData(
            previous_time, current_time, &how_many_new_slices, &app_feed);
        if (feature_status != kTfLiteOk)
        {
            ESP_LOGE(TAG, "Feature generation failed");
            return false;
        }

        previous_time = current_time;
        // If no new audio samples have been received since last time, don't bother
        // running the network model.
        if (how_many_new_slices > 0)
        {
            // Flatten the spectrogram into the model's int8 input tensor.
            for (int i = 0; i < kFeatureElementCount; i++)
            {
                model_input_buffer[i] = feature_buffer[i];
            }

            // Execute one inference pass on the current spectrogram frame stack.
            auto do_invoke = [&]() -> bool
            {
                TfLiteStatus invoke_status = interpreter->Invoke();
                if (invoke_status != kTfLiteOk)
                {
                    ESP_LOGE(TAG, "Invoke failed");
                    return false;
                }
                return true;
            };

            if (!APP_RUN_WITH_TIMING(TAG, do_invoke()))
            {
                return false;
            }
        }

        return true;
    }

    /// Reads the model output tensor, dequantizes scores, logs the best
    /// keyword result, and resets the feature buffer for the next clip.
    bool handleResult2()
    {
        // Read the quantized output scores produced by the model.
        TfLiteTensor* output = interpreter->output(0);

        float output_scale = output->params.scale; // Q → float multiplier baked in model
        int output_zero_point =
            output->params.zero_point; // int8 value that maps to 0.0 in float space
        int max_idx = 0;               // index of the highest-scoring category so far
        float max_result = 0.0;        // dequantized score of that category (0.0–1.0)

        // Dequantize each class score and keep the best-scoring label.
        for (int i = 0; i < kCategoryCount; i++)
        {
            float current_result =
                (tflite::GetTensorData<int8_t>(output)[i] - output_zero_point) * output_scale;
            if (current_result > max_result)
            {
                max_result = current_result; // update max result
                max_idx = i;                 // update category
            }
        }

        if (max_result > detection_threshold)
        {
            ESP_LOGI(TAG, ">>> Detected %7s, score: %.2f", kCategoryLabels[max_idx],
                     static_cast<double>(max_result));

            if (max_idx == 3) // "no" detected
            {
                ESP_LOGI(TAG, "NO detected!");
                return true;
            }
        }
        else
        {
            ESP_LOGI(TAG, "No keyword detected, max score: %.2f", static_cast<double>(max_result));
        }

        // Start the next clip with a cleared spectrogram history.
        feature_provider->reset();
        return false;
    }

  private:
    static constexpr const char* TAG = "inference";

    // Parsed flatbuffer speech model stored in flash.
    const tflite::Model* model = nullptr;

    // TFLite Micro interpreter bound to the speech model and tensor arena.
    tflite::MicroInterpreter* interpreter = nullptr;

    // Input tensor that receives the flattened spectrogram features.
    TfLiteTensor* model_input = nullptr;

    // Feature extractor that maintains the rolling spectrogram state.
    AppFeatures* feature_provider = nullptr;

    // Offline audio source that serves the bundled yes/no clips.
    AppFeed app_feed;

    // Scratch arena used by TFLite Micro for tensors and intermediate buffers.
    uint8_t tensor_arena[TENSOR_ARENA_SIZE];

    // Rolling spectrogram buffer written by AppFeatures.
    int8_t feature_buffer[kFeatureElementCount];

    // Raw pointer to the input tensor payload for fast writes before invoke.
    int8_t* model_input_buffer = nullptr;

    // Timestamp of the previous feature-generation step in milliseconds.
    int32_t previous_time = 0;

    // threshold for logging a detected keyword
    static constexpr float detection_threshold = CONFIG_DETECTION_THRESHOLD / 100.0f;
};
} // namespace app
