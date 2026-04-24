#pragma once

#include "AppFeedBase.hpp"

namespace app
{

    class AppInferenceBase
    {
    public:
        virtual ~AppInferenceBase() = default;

        // Initialize the inference engine, load the model, etc.
        virtual void init() = 0;

        // Feed data into the inference engine, e.g., audio samples, images, etc.
        virtual bool feed(const raw_data_t *const data) = 0;

        // Run the inference engine on the fed data and produce results.
        virtual bool run() = 0;

        // Handle the inference results, e.g., trigger actions based on detected keywords, etc.
        virtual void handleResult() = 0;
    };

} // namespace app