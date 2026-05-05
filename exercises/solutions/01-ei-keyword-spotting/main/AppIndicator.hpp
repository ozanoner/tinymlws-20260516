
#pragma once

#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "led_indicator_gpio.h"

namespace app
{

class AppIndicator
{
  public:
    virtual ~AppIndicator() = default;

    void init()
    {
        esp_err_t ret = led_indicator_new_gpio_device(&config, &gpio_config, &led_handle);
        ESP_ERROR_CHECK(ret);
        assert(led_handle != nullptr);
    }

    void blink()
    {
        led_indicator_start(led_handle, BLINK_DOUBLE);
    }

  private:
    static constexpr int32_t BLINK_GPIO{17};

    led_indicator_handle_t led_handle{nullptr};

    enum
    {
        BLINK_DOUBLE = 0,
        BLINK_TRIPLE,
        BLINK_MAX,
    };

    static constexpr blink_step_t double_blink[] = {
        {LED_BLINK_HOLD, LED_STATE_ON, 200},
        {LED_BLINK_HOLD, LED_STATE_OFF, 200},
        {LED_BLINK_HOLD, LED_STATE_ON, 200},
        {LED_BLINK_HOLD, LED_STATE_OFF, 200},
        {LED_BLINK_STOP, 0, 0},
    };

    static constexpr blink_step_t triple_blink[] = {
        {LED_BLINK_HOLD, LED_STATE_ON, 200},
        {LED_BLINK_HOLD, LED_STATE_OFF, 200},
        {LED_BLINK_HOLD, LED_STATE_ON, 200},
        {LED_BLINK_HOLD, LED_STATE_OFF, 200},
        {LED_BLINK_HOLD, LED_STATE_ON, 200},
        {LED_BLINK_HOLD, LED_STATE_OFF, 200},
        {LED_BLINK_STOP, 0, 0},
    };

    inline static const blink_step_t* led_mode[] = {double_blink, triple_blink, nullptr};

    led_indicator_gpio_config_t gpio_config = {
        .is_active_level_high = true,
        .gpio_num = BLINK_GPIO,
    };

    const led_indicator_config_t config = {
        .blink_lists = led_mode,
        .blink_list_num = BLINK_MAX,
    };
};

} // namespace app