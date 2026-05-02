
// Entry point for the Edge Impulse keyword-spotting demo.
// Iterates over pre-recorded PCM clips via AppFeed, runs the EI classifier
// through AppInference for each clip, and prints the label scores.

#include "esp_log.h"
#include "sdkconfig.h"

#include "AppFeed.hpp"
#include "AppInference.hpp"

static constexpr const char *TAG = "kws";

namespace
{
    app::AppInference inference;
    app::AppFeed feed;
}

extern "C" void app_main()
{
    feed.init();
    inference.init();

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
    }
}
