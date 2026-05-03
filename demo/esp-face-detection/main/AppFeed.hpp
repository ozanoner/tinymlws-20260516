// AppFeed — image source for the face-detection pipeline.
// Exposes the five bundled 160x120 RGB888 frames (data/offline_frames.hpp)
// one at a time via next().

#pragma once

#include "offline_frames.hpp"
#include "AppFeedBase.hpp"

namespace app
{
    class AppFeed : public AppFeedBase<uint8_t>
    {
    public:
        virtual ~AppFeed() = default;
        void init() override
        {
        }
        const raw_data_t<uint8_t> *next() override
        {
            ++current_index;
            if (current_index >= OFFLINE_FRAME_COUNT)
            {
                return nullptr;
            }
            return &frames[current_index];
        }

    private:
        static constexpr size_t frame_size = OFFLINE_SRC_W * OFFLINE_SRC_H * 3;
        static constexpr raw_data_t<uint8_t> frames[OFFLINE_FRAME_COUNT] = {
            {kOfflineFrameRgb[0], frame_size},
            {kOfflineFrameRgb[1], frame_size},
            {kOfflineFrameRgb[2], frame_size},
            {kOfflineFrameRgb[3], frame_size},
            {kOfflineFrameRgb[4], frame_size},
        };

        int current_index = -1;
    };

}