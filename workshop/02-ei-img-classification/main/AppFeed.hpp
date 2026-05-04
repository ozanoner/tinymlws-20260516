
// AppFeed — image data source for the EI image classification pipeline.
// Exposes bundled test images (offline_sample.h) one at a time via next().

#pragma once

#include "AppFeedBase.hpp"
#include "lamp-1.png.h"
#include "offline_sample.h"
#include "plant-1.png.h"
#include "plant-2.png.h"

namespace app
{
class AppFeed : public AppFeedBase<uint32_t>
{
  public:
    virtual ~AppFeed() = default;
    void init() override {}
    const raw_data_t<uint32_t>* next() override
    {
        ++current_index;
        if (current_index >= data_len)
        {
            return nullptr;
        }
        return &data[current_index];
    }

    const char* getExpectedLabel()
    {
        if (current_index >= data_len)
        {
            return nullptr;
        }
        return expected_labels[current_index];
    }

  private:
    static constexpr const char* TAG = "feed";

    static constexpr raw_data_t<uint32_t> data[] = {
        {features_p0, sizeof(features_p0) / sizeof(features_p0[0])},
        {features_plant1, sizeof(features_plant1) / sizeof(features_plant1[0])},
        {features_plant2, sizeof(features_plant2) / sizeof(features_plant2[0])},
        {features_lamp1, sizeof(features_lamp1) / sizeof(features_lamp1[0])},
    };
    static const size_t data_len = sizeof(data) / sizeof(data[0]);

    static constexpr const char* expected_labels[] = {
        "plant",
        "plant",
        "plant",
        "lamp",
    };

    int current_index = -1;
};

} // namespace app