#pragma once
#include <cstdint>
#include <cstddef>

namespace app
{
    struct raw_data_t
    {
        const int16_t *data;
        size_t length;
    };

    class AppFeedBase
    {
    public:
        virtual ~AppFeedBase() = default;

        virtual void init() = 0;
        virtual const raw_data_t *next() = 0;
    };

} // namespace app