#include "offline_frames.hpp"

#include "dl_image.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "human_face_detect.hpp"

namespace
{
    constexpr const char *TAG = "OfflineDetect";
    constexpr TickType_t FRAME_DELAY_TICKS = pdMS_TO_TICKS(120);

    const char *to_human_label(bool has_human)
    {
        return has_human ? "human" : "non-human";
    }

    void log_detection_result(size_t frame_idx, const std::list<dl::detect::result_t> &result)
    {
        const bool predicted_has_human = !result.empty();
        const bool expected_has_human = kOfflineFrameHasHuman[frame_idx];

        ESP_LOGI(TAG,
                 "frame=%u predicted=%s expected=%s detections=%u",
                 static_cast<unsigned>(frame_idx),
                 to_human_label(predicted_has_human),
                 to_human_label(expected_has_human),
                 static_cast<unsigned>(result.size()));
    }
} // namespace

extern "C" void app_main(void)
{
    auto *model = new HumanFaceDetect(
        static_cast<HumanFaceDetect::model_type_t>(CONFIG_DEFAULT_HUMAN_FACE_DETECT_MODEL), false);
    model->set_score_thr(0.3F, 0);
    model->set_score_thr(0.3F, 1);

    while (true)
    {
        for (size_t frame_idx = 0; frame_idx < OFFLINE_FRAME_COUNT; ++frame_idx)
        {
            dl::image::img_t img = {
                .data = const_cast<uint8_t *>(kOfflineFrameRgb[frame_idx]),
                .width = OFFLINE_SRC_W,
                .height = OFFLINE_SRC_H,
                .pix_type = dl::image::DL_IMAGE_PIX_TYPE_RGB888,
            };

            auto &result = model->run(img);
            log_detection_result(frame_idx, result);
            vTaskDelay(FRAME_DELAY_TICKS);
        }
    }
}
