
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
        if (!inference.run())
        {
            ESP_LOGE(TAG, "Failed to run inference");
            break;
        }
        inference.handleResult();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
