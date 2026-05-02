
// AppFeed — image data source for the EI image classification pipeline.
// Exposes bundled test images (offline_sample.h) one at a time via next().

#pragma once

#include "offline_sample.h"
#include "AppFeedBase.hpp"

namespace app
{
    class AppFeed : public AppFeedBase<uint32_t>
    {
    public:
        virtual ~AppFeed() = default;
        void init() override
        {
        }
        const raw_data_t<uint32_t> *next() override
        {
            if (current_index >= data_len)
            {
                return nullptr;
            }
            return &data[current_index++];
        }

    private:
        static constexpr raw_data_t<uint32_t> data[] = {
            {features, sizeof(features) / sizeof(features[0])}};
        static const size_t data_len = sizeof(data) / sizeof(data[0]);

        size_t current_index = 0;
    };

}