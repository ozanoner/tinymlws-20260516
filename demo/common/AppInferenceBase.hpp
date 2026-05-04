#pragma once

#include <cstdint>

#include "esp_log.h"
#include "esp_timer.h"

#include "AppFeedBase.hpp"

#define APP_RUN_WITH_TIMING(TAG, RUN_CALL) \
    ([&]() -> bool {                                                                 \
        const int64_t _app_run_start_us = esp_timer_get_time();                      \
        const bool _app_run_ok = (RUN_CALL);                                         \
        const int64_t _app_run_elapsed_ms =                                          \
            (esp_timer_get_time() - _app_run_start_us) / 1000;                       \
        if (!_app_run_ok)                                                            \
        {                                                                            \
            ESP_LOGW((TAG), "run() failed",);                                        \
        }                                                                            \
        else                                                                         \
        {                                                                            \
            ESP_LOGI((TAG), "run() took %lld ms",                                    \
                     static_cast<long long>(_app_run_elapsed_ms));                   \
        }                                                                            \
        return _app_run_ok; }())

namespace app
{

    template <typename T>
    class AppInferenceBase
    {
    public:
        virtual ~AppInferenceBase() = default;

        // Initialize the inference engine, load the model, etc.
        virtual void init() {}

        // Feed data into the inference engine, e.g., audio samples, images, etc.
        virtual bool feed(const raw_data_t<T> *const data) { return false; }

        // Run the inference engine on the fed data and produce results.
        virtual bool run() = 0;

        // Handle the inference results, e.g., trigger actions based on detected keywords, etc.
        virtual void handleResult() {}
    };

} // namespace app