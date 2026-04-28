#pragma once

#include <algorithm>
#include "esp_log.h"
#include "esp_heap_caps.h"

#include "model_settings.h"
#include "person_detect_model_data.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "AppInferenceBase.hpp"

namespace app
{
    class AppInference : public AppInferenceBase<int8_t>
    {
    public:
        // Initialize the inference engine, load the model, etc.
        void init() override
        {
            model = tflite::GetModel(g_person_detect_model_data);
            if (model->version() != TFLITE_SCHEMA_VERSION)
            {
                ESP_LOGE(TAG, "Model schema version %d does not match supported version %d",
                         model->version(), TFLITE_SCHEMA_VERSION);
                return;
            }

            if (tensor_arena == NULL)
            {
                tensor_arena = (uint8_t *)heap_caps_malloc(kTensorArenaSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
            }
            if (tensor_arena == NULL)
            {
                ESP_LOGE(TAG, "Couldn't allocate memory of %d bytes", kTensorArenaSize);
                return;
            }

            static tflite::MicroMutableOpResolver<5> micro_op_resolver;
            micro_op_resolver.AddAveragePool2D();
            micro_op_resolver.AddConv2D();
            micro_op_resolver.AddDepthwiseConv2D();
            micro_op_resolver.AddReshape();
            micro_op_resolver.AddSoftmax();

            static tflite::MicroInterpreter static_interpreter(
                model, micro_op_resolver, tensor_arena, kTensorArenaSize);
            interpreter = &static_interpreter;

            TfLiteStatus allocate_status = interpreter->AllocateTensors();
            if (allocate_status != kTfLiteOk)
            {
                ESP_LOGE(TAG, "AllocateTensors() failed");
                return;
            }

            input = interpreter->input(0);
        }
        // Feed data into the inference engine, e.g., audio samples, images, etc.
        bool feed(const raw_data_t<int8_t> *const data) override
        {
            if (data == nullptr || data->length != (kNumCols * kNumRows))
            {
                return false;
            }
            current_data = data;
            return true;
        }

        // Run the inference engine on the fed data and produce results.
        bool run() override
        {
            std::copy(current_data->data, current_data->data + current_data->length, input->data.uint8);

            // Run the model on this input and make sure it succeeds.
            if (kTfLiteOk != interpreter->Invoke())
            {
                ESP_LOGE(TAG, "Invoke failed");
                return false;
            }
            return true;
        }

        // Handle the inference results, e.g., trigger actions based on detected keywords, etc.
        void handleResult() override
        {
            TfLiteTensor *output = interpreter->output(0);

            // Process the inference results.
            int8_t person_score = output->data.uint8[kPersonIndex];
            int8_t no_person_score = output->data.uint8[kNotAPersonIndex];

            float person_score_f =
                (person_score - output->params.zero_point) * output->params.scale;
            float no_person_score_f =
                (no_person_score - output->params.zero_point) * output->params.scale;

            ESP_LOGI(TAG, "Person score: %f, No person score: %f", person_score_f, no_person_score_f);
        }

    protected:
        static constexpr const char *TAG = "inference";

        const tflite::Model *model = nullptr;
        tflite::MicroInterpreter *interpreter = nullptr;
        TfLiteTensor *input = nullptr;

        // In order to use optimized tensorflow lite kernels, a signed int8_t quantized
        // model is preferred over the legacy unsigned model format. This means that
        // throughout this project, input images must be converted from unisgned to
        // signed format. The easiest and quickest way to convert from unsigned to
        // signed 8-bit integers is to subtract 128 from the unsigned value to get a
        // signed value.

        // CONFIG_NN_OPTIMIZED
        const int scratchBufSize = 60 * 1024;
        // An area of memory to use for input, output, and intermediate arrays.
        // Keeping allocation on bit larger size to accomodate future needs.
        const int kTensorArenaSize = 100 * 1024 + scratchBufSize;
        uint8_t *tensor_arena; //[kTensorArenaSize]; // Maybe we should move this to external

        const raw_data_t<int8_t> *current_data{nullptr};
    };
}