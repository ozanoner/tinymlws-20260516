#pragma once

#include "esp_log.h"
#include <algorithm>
#include <cstdint>
#include <iterator>

#include "audio_provider.h"
#include "command_responder.h"
#include "feature_provider.h"
#include "micro_model_settings.h"
#include "model.h"
#include "recognize_commands.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/core/c/common.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"

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
            current_data = nullptr;

            // Map the model into a usable data structure. This doesn't involve any
            // copying or parsing, it's a very lightweight operation.
            model = tflite::GetModel(g_model);
            if (model->version() != TFLITE_SCHEMA_VERSION)
            {
                MicroPrintf("Model provided is schema version %d not equal to supported "
                            "version %d.",
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
                MicroPrintf("AllocateTensors() failed");
                return;
            }

            // Get information about the memory area to use for the model's input.
            model_input = interpreter->input(0);
            if ((model_input->dims->size != 2) || (model_input->dims->data[0] != 1) ||
                (model_input->dims->data[1] !=
                 (kFeatureCount * kFeatureSize)) ||
                (model_input->type != kTfLiteInt8))
            {
                MicroPrintf("Bad input tensor parameters in model");
                return;
            }
            model_input_buffer = tflite::GetTensorData<int8_t>(model_input);

            // Prepare to access the audio spectrograms from a microphone or other source
            // that will provide the inputs to the neural network.
            // NOLINTNEXTLINE(runtime-global-variables)
            static FeatureProvider static_feature_provider(kFeatureElementCount,
                                                           feature_buffer);
            feature_provider = &static_feature_provider;

            static RecognizeCommands static_recognizer;
            recognizer = &static_recognizer;

            previous_time = 0;
        }

        bool feed(const raw_data_t<int16_t> *const data) override
        {
            if (data == nullptr)
            {
                return false;
            }
            current_data = data;
            return true;
        }

        bool run() override
        {

            return true;
        }

        void handleResult() override
        {
        }

    private:
        static constexpr const char *TAG = "inference";

        const raw_data_t<int16_t> *current_data{nullptr};

        const tflite::Model *model = nullptr;
        tflite::MicroInterpreter *interpreter = nullptr;
        TfLiteTensor *model_input = nullptr;
        FeatureProvider *feature_provider = nullptr;
        RecognizeCommands *recognizer = nullptr;
        int32_t previous_time = 0;

        // Create an area of memory to use for input, output, and intermediate arrays.
        // The size of this will depend on the model you're using, and may need to be
        // determined by experimentation.
        static const int kTensorArenaSize = 30 * 1024;
        uint8_t tensor_arena[kTensorArenaSize];
        int8_t feature_buffer[kFeatureElementCount];
        int8_t *model_input_buffer = nullptr;
    };
}
