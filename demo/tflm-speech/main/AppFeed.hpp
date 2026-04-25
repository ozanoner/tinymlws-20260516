
#pragma once

#include "AppFeedBase.hpp"

namespace app
{
    class AppFeed : public AppFeedBase<int16_t>
    {
    public:
        virtual ~AppFeed() = default;
        void init() override
        {
        }
        const raw_data_t<int16_t> *next() override
        {
            if (current_index >= data_len)
            {
                return &no_data; // No more data
            }
            return &data[current_index++];
        }

    private:
        static constexpr raw_data_t<int16_t> data[] = {};
        static const size_t data_len = sizeof(data) / sizeof(data[0]);

        static constexpr raw_data_t<int16_t> no_data{nullptr, 0};

        size_t current_index = 0;
    };

}