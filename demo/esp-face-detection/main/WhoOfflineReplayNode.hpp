#include "offline_frames.hpp"
#include "who_frame_cap.hpp"
// #include "who_cam.hpp"
#include <cstring>
#include <sys/time.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

using namespace who::cam;
using namespace who::frame_cap;

// num of frames the model take to get result
#define MODEL_TIME 2
#define MODEL_INPUT_W 160
#define MODEL_INPUT_H 120

namespace app
{
    static constexpr uint16_t OFFLINE_FRAME_W = 160;
    static constexpr uint16_t OFFLINE_FRAME_H = 120;
    static constexpr uint32_t OFFLINE_REPLAY_LOG_PERIOD = 10;

    class WhoOfflineReplayNode : public WhoFrameCapNode
    {
    public:
        WhoOfflineReplayNode(const std::string &name) : WhoFrameCapNode(name, MODEL_TIME + 2),
                                                        m_frame_idx(0),
                                                        m_generated_frames(0)
        {
        }

        uint16_t get_fb_width() override { return OFFLINE_FRAME_W; }
        uint16_t get_fb_height() override { return OFFLINE_FRAME_H; }
        std::string get_type() override { return "OfflineReplayNode"; }

    private:
        void cleanup() override
        {
            xSemaphoreTake(m_mutex, portMAX_DELAY);
            while (!m_cam_fbs.empty())
            {
                auto fb = m_cam_fbs.pop();
                heap_caps_free(fb->buf);
                delete fb;
            }
            xSemaphoreGive(m_mutex);
        }

        who::cam::cam_fb_t *process(who::cam::cam_fb_t *fb) override
        {
            (void)fb;
            size_t buf_size = OFFLINE_FRAME_W * OFFLINE_FRAME_H * 3U;
            auto *buf = static_cast<uint8_t *>(heap_caps_malloc(buf_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
            if (!buf)
            {
                buf = static_cast<uint8_t *>(heap_caps_malloc(buf_size, MALLOC_CAP_8BIT));
            }
            if (!buf)
            {
                ESP_LOGE("OfflineReplayNode", "alloc frame buffer failed");
                return nullptr;
            }

            const uint8_t *src = kOfflineFrameRgb[m_frame_idx];
            std::memcpy(buf, src, buf_size);

            dl::image::img_t img = {
                .data = buf,
                .width = OFFLINE_FRAME_W,
                .height = OFFLINE_FRAME_H,
                .pix_type = dl::image::DL_IMAGE_PIX_TYPE_RGB888,
            };
            timeval tv;
            gettimeofday(&tv, nullptr);
            ++m_generated_frames;
            m_frame_idx = (m_frame_idx + 1) % OFFLINE_FRAME_COUNT;
            vTaskDelay(pdMS_TO_TICKS(120));
            return new who::cam::cam_fb_t(img, tv);
        }

        void update_ringbuf(who::cam::cam_fb_t *fb) override
        {
            if (m_cam_fbs.full())
            {
                auto fb_prev = m_cam_fbs.pop();
                heap_caps_free(fb_prev->buf);
                delete fb_prev;
            }
            m_cam_fbs.push(fb);
        }

        size_t m_frame_idx;
        size_t m_generated_frames;
    };
} // namespace