
#pragma once

#include "AppFeedBase.hpp"
#include "data/gm_sample.wav.hpp"
#include "data/unknown.6mq9gm71.wav.hpp"

namespace app
{
class AppFeed : public AppFeedBase<int16_t>
{
  public:
    virtual ~AppFeed() = default;
    void init() override {}
    const raw_data_t<int16_t>* next() override
    {
        ++current_index;
        if (current_index >= data_cnt)
        {
            return nullptr;
        }
        return &data[current_index];
    }

  private:
    static constexpr raw_data_t<int16_t> data[] = {
        {kOfflineKeywordSample_gm, kOfflineKeywordSampleLength_gm},
        {kOfflineKeywordSample_unknown, kOfflineKeywordSampleLength_unknown}};
    static const size_t data_cnt = sizeof(data) / sizeof(data[0]);

    int current_index = -1;
};

} // namespace app