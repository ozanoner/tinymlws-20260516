// Entry point for the TFLM speech demo.
// Repeatedly runs offline keyword spotting over bundled yes/no audio clips.

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "AppInference.hpp"

static constexpr const char* TAG = "kws";

namespace
{
app::AppInference inference;
}

extern "C" void app_main()
{
    inference.init();

    while (inference.run())
    {
        bool no_detected = inference.handleResult2();
        if (no_detected)
        {
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
