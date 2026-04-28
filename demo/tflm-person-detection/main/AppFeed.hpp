#pragma once

#include <algorithm>
#include "esp_log.h"

#include "AppFeedBase.hpp"

#define IMAGE_COUNT 10
#define IMAGE_SIZE (96 * 96)

extern const uint8_t image0_start[] asm("_binary_image0_start");
extern const uint8_t image1_start[] asm("_binary_image1_start");
extern const uint8_t image2_start[] asm("_binary_image2_start");
extern const uint8_t image3_start[] asm("_binary_image3_start");
extern const uint8_t image4_start[] asm("_binary_image4_start");
extern const uint8_t image5_start[] asm("_binary_image5_start");
extern const uint8_t image6_start[] asm("_binary_image6_start");
extern const uint8_t image7_start[] asm("_binary_image7_start");
extern const uint8_t image8_start[] asm("_binary_image8_start");
extern const uint8_t image9_start[] asm("_binary_image9_start");

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

            std::transform(image_database[current_index], image_database[current_index] + IMAGE_SIZE, current_image, [](int x)
                           { return x ^ 0x80; });
            return &data;
        }

    private:
        static constexpr const char *TAG = "feed";
        const uint8_t *image_database[IMAGE_COUNT]{
            image0_start, image1_start, image2_start,
            image3_start, image4_start, image5_start,
            image6_start, image7_start, image8_start,
            image9_start};

        int8_t current_image[IMAGE_SIZE]{};
        raw_data_t<int8_t> data{current_image, IMAGE_SIZE};
        int current_index = -1;
    };
}
