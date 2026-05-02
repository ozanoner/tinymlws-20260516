
// Entry point for the Edge Impulse image classification demo.
// Iterates over bundled test images via AppFeed, runs the EI classifier
// through AppInference for each image, and reports the top prediction.

#include "esp_log.h"

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

    while (const app::raw_data_t<uint32_t> *data = feed.next())
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
    }
}
