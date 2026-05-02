#pragma once

#include <list>
#include "esp_log.h"
#include "dl_image.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "human_face_detect.hpp"

#include "AppInferenceBase.hpp"

namespace app
{
    class AppInference : public AppInferenceBase<uint8_t>
    {
    public:
        AppInference() = default;
        ~AppInference() override
        {
            if (model != nullptr)
            {
                delete model;
                model = nullptr;
            }
        }

        void init() override
        {
            model = new HumanFaceDetect(
                static_cast<HumanFaceDetect::model_type_t>(CONFIG_DEFAULT_HUMAN_FACE_DETECT_MODEL), false);
            model->set_score_thr(0.7F, 0);
            model->set_score_thr(0.7F, 1);
        }

        bool feed(const raw_data_t<uint8_t> *const data) override
        {
            if (data == nullptr || data->data == nullptr || data->length == 0)
            {
                return false;
            }
            current_data = data;
            return true;
        }

        bool run() override
        {
            if (current_data == nullptr)
            {
                ESP_LOGW(TAG, "No data to run inference");
                return false;
            }

            const dl::image::img_t img = {
                .data = const_cast<uint8_t *>(current_data->data),
                .width = OFFLINE_SRC_W,
                .height = OFFLINE_SRC_H,
                .pix_type = dl::image::DL_IMAGE_PIX_TYPE_RGB888,
            };
            result = model->run(img);

            return true;
        }

        void handleResult() override
        {
            if (result.empty())
            {
                ESP_LOGI(TAG, "No detection");
                return;
            }

            for (const auto &res : result)
            {
                ESP_LOGI(TAG,
                         "Detection: label=%u score=%.2f",
                         res.category,
                         res.score);
            }
        }

    private:
        static constexpr const char *TAG = "inference";

        HumanFaceDetect *model{nullptr};
        const raw_data_t<uint8_t> *current_data{nullptr};
        std::list<dl::detect::result_t> result;
    };
}
