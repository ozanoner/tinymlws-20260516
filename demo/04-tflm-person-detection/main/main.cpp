// Entry point for the TFLM person-detection demo.
// Iterates over bundled grayscale frames via AppFeed and classifies each
// using AppInference (TFLite Micro person_detect model).

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "AppFeed.hpp"
#include "AppInference.hpp"

static constexpr const char *TAG = "img";

namespace
{
    app::AppInference inference;
    app::AppFeed feed;
}

extern "C" void app_main()
{
    feed.init();
    inference.init();

    while (const app::raw_data_t<int8_t> *data = feed.next())
    {
        if (!inference.feed(data))
        {
            ESP_LOGW(TAG, "No more data to feed");
            break;
        }
        if (!APP_RUN_WITH_TIMING(TAG, inference.run()))
        {
            ESP_LOGE(TAG, "Failed to run inference");
            break;
        }
        inference.handleResult();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
