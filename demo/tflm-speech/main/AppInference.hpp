#pragma once

#include "esp_log.h"
#include <algorithm>
#include <cstdint>
#include <iterator>

#include "micro_model_settings.h"
#include "model.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/core/c/common.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"

#include "AppInferenceBase.hpp"
#include "AppFeatures.hpp"
#include "AppFeed.hpp"

namespace app
{
    class AppInference : public AppInferenceBase<int16_t>
    {
    public:
        AppInference() = default;
        ~AppInference() override = default;

        void init() override
        {
            current_data = nullptr;

            // Map the model into a usable data structure. This doesn't involve any
            // copying or parsing, it's a very lightweight operation.
            model = tflite::GetModel(g_model);
            if (model->version() != TFLITE_SCHEMA_VERSION)
            {
                ESP_LOGE(TAG,
                         "Model schema version %d does not match supported version %d",
                         model->version(), TFLITE_SCHEMA_VERSION);
                return;
            }

            // Pull in only the operation implementations we need.
            // This relies on a complete list of all the ops needed by this graph.
            // An easier approach is to just use the AllOpsResolver, but this will
            // incur some penalty in code space for op implementations that are not
            // needed by this graph.
            //
            // tflite::AllOpsResolver resolver;
            // NOLINTNEXTLINE(runtime-global-variables)
            static tflite::MicroMutableOpResolver<4> micro_op_resolver;
            if (micro_op_resolver.AddDepthwiseConv2D() != kTfLiteOk)
            {
                return;
            }
            if (micro_op_resolver.AddFullyConnected() != kTfLiteOk)
            {
                return;
            }
            if (micro_op_resolver.AddSoftmax() != kTfLiteOk)
            {
                return;
            }
            if (micro_op_resolver.AddReshape() != kTfLiteOk)
            {
                return;
            }

            // Build an interpreter to run the model with.
            static tflite::MicroInterpreter static_interpreter(
                model, micro_op_resolver, tensor_arena, kTensorArenaSize);
            interpreter = &static_interpreter;

            // Allocate memory from the tensor_arena for the model's tensors.
            TfLiteStatus allocate_status = interpreter->AllocateTensors();
            if (allocate_status != kTfLiteOk)
            {
                ESP_LOGE(TAG, "AllocateTensors() failed");
                return;
            }

            // Get information about the memory area to use for the model's input.
            model_input = interpreter->input(0);
            if ((model_input->dims->size != 2) || (model_input->dims->data[0] != 1) ||
                (model_input->dims->data[1] !=
                 (kFeatureCount * kFeatureSize)) ||
                (model_input->type != kTfLiteInt8))
            {
                ESP_LOGE(TAG, "Bad input tensor parameters in model");
                return;
            }
            model_input_buffer = tflite::GetTensorData<int8_t>(model_input);

            // Prepare to access the audio spectrograms from a microphone or other source
            // that will provide the inputs to the neural network.
            static AppFeatures static_feature_provider(kFeatureElementCount,
                                                       feature_buffer);
            feature_provider = &static_feature_provider;

            app_feed.init();

            previous_time = 0;
        }

        bool feed(const raw_data_t<int16_t> *const data) override { return false; }

        bool run() override
        {
            auto next_data = app_feed.next();
            if (next_data == nullptr || next_data->data == nullptr || next_data->length == 0)
            {
                ESP_LOGW(TAG, "No more data to feed");
                return false;
            }

            // Fetch the spectrogram for the current time.
            const int32_t current_time = app_feed.LatestAudioTimestamp();

            int how_many_new_slices = 0;
            TfLiteStatus feature_status = feature_provider->PopulateFeatureData(
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

                // Copy feature buffer to input tensor
                for (int i = 0; i < kFeatureElementCount; i++)
                {
                    model_input_buffer[i] = feature_buffer[i];
                }

                // Run the model on the spectrogram input and make sure it succeeds.
                TfLiteStatus invoke_status = interpreter->Invoke();
                if (invoke_status != kTfLiteOk)
                {
                    ESP_LOGE(TAG, "Invoke failed");
                    return false;
                }
            }

            return true;
        }

        void handleResult() override
        {
            // Obtain a pointer to the output tensor
            TfLiteTensor *output = interpreter->output(0);

            float output_scale = output->params.scale;
            int output_zero_point = output->params.zero_point;
            int max_idx = 0;
            float max_result = 0.0;
            // Dequantize output values and find the max
            for (int i = 0; i < kCategoryCount; i++)
            {
                float current_result =
                    (tflite::GetTensorData<int8_t>(output)[i] - output_zero_point) *
                    output_scale;
                if (current_result > max_result)
                {
                    max_result = current_result; // update max result
                    max_idx = i;                 // update category
                }
            }
            if (max_result > 0.8f)
            {
                ESP_LOGI(TAG, "Detected %7s, score: %.2f", kCategoryLabels[max_idx],
                         static_cast<double>(max_result));
            }

            feature_provider->reset();
        }

    private:
        static constexpr const char *TAG = "inference";

        const raw_data_t<int16_t> *current_data{nullptr};

        const tflite::Model *model = nullptr;
        tflite::MicroInterpreter *interpreter = nullptr;
        TfLiteTensor *model_input = nullptr;
        AppFeatures *feature_provider = nullptr;
        int32_t previous_time = 0;

        // Create an area of memory to use for input, output, and intermediate arrays.
        // The size of this will depend on the model you're using, and may need to be
        // determined by experimentation.
        static const int kTensorArenaSize = 30 * 1024;
        uint8_t tensor_arena[kTensorArenaSize];
        int8_t feature_buffer[kFeatureElementCount];
        int8_t *model_input_buffer = nullptr;

        AppFeed app_feed;
    };
}
