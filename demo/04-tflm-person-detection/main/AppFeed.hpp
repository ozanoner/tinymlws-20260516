// AppFeed — image source for the TFLM person-detection pipeline.
// Exposes bundled 96x96 grayscale C++ headers one at a time via next(),
// converting each pixel from uint8 to int8 for the model input.

#pragma once

#include <algorithm>
#include "esp_log.h"

#include "cat-1.png.h"
#include "chimp-1.png.h"
#include "human-1.png.h"
#include "human-2.png.h"
#include "human-3.png.h"

#include "AppFeedBase.hpp"

#define IMAGE_COUNT 3
#define IMAGE_SIZE (96 * 96)

namespace app
{
    class AppFeed : public AppFeedBase<int8_t>
    {
    public:
        virtual ~AppFeed() = default;

        void init() override
        {
        }

        const raw_data_t<int8_t> *next() override
        {
            ++current_index;
            if (current_index >= IMAGE_COUNT)
            {
                return nullptr;
            }

            // from uint8 to int8 conversion for model input
            std::transform(image_database[current_index], image_database[current_index] + IMAGE_SIZE, current_image, [](int x)
                           { return x ^ 0x80; });
            return &data;
        }

    private:
        static constexpr const char *TAG = "feed";
        const uint8_t *image_database[IMAGE_COUNT]{
            image_data1,
            image_data2,
            image_data3,
        };

        int8_t current_image[IMAGE_SIZE]{};
        raw_data_t<int8_t> data{current_image, IMAGE_SIZE};
        int current_index = -1;
    };
}
