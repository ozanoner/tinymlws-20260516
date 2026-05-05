
// AppFeed — image data source for the EI image classification pipeline.
// Exposes bundled test images (offline_sample.h) one at a time via next().

#pragma once

#include "AppFeedBase.hpp"
#include "human-1.png.h"
#include "people-1.png.h"
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
        {features_human1, sizeof(features_human1) / sizeof(features_human1[0])},
        {features_people1, sizeof(features_people1) / sizeof(features_people1[0])},
        {features_plant2, sizeof(features_plant2) / sizeof(features_plant2[0])},
    };
    static const size_t data_len = sizeof(data) / sizeof(data[0]);

    static constexpr const char* expected_labels[] = {
        "human",
        "plant",
    };

    int current_index = -1;
};

} // namespace app