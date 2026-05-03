// Entry point for the ESP face-detection demo.
// Iterates over bundled RGB frames via AppFeed and detects human faces
// using AppInference (HumanFaceDetect / ESP-DL).

#include "esp_log.h"
#include "sdkconfig.h"

#include "AppFeed.hpp"
#include "AppInference.hpp"

static constexpr const char *TAG = "app";

namespace
{
    app::AppInference inference;
    app::AppFeed feed;
}

extern "C" void app_main()
{
    feed.init();
    inference.init();

    while (const app::raw_data_t<uint8_t> *data = feed.next())
    {
        ESP_LOGI(TAG, ">> Feeding data to inference");
        if (!inference.feed(data))
        {
            ESP_LOGW(TAG, "No more data to feed");
            break;
        }
        if (!inference.run())
        {
            ESP_LOGE(TAG, "Failed to run inference");
            break;
        }
        inference.handleResult();
    }
}
