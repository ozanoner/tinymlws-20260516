
#include "esp_log.h"
#include "sdkconfig.h"

#include "AppInference.hpp"

static constexpr const char *TAG = "kws";

namespace
{
    app::AppInference inference;
}

extern "C" void app_main()
{
    inference.init();

    while (inference.run())
    {
        inference.handleResult();
    }
}
