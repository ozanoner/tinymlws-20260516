
// Entry point for the ESP-SR wake-word detection demo.
// Streams pre-recorded PCM audio via AppFeed and detects the "Hi ESP" wake word
// using AppInference (WakeNet wn9s_hiesp).

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "AppFeed.hpp"
#include "AppInference.hpp"

namespace
{
    app::AppInference inference;
    app::AppFeed feed;
    constexpr const char *TAG = "app";
}

extern "C" void app_main(void)
{
    inference.init();
    feed.init(inference.getAudioChunkSize());

    while (const app::raw_data_t<int16_t> *data = feed.next())
    {
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
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
