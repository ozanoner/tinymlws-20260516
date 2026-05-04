#pragma once
#include <cstdint>
#include <cstddef>

namespace app
{
    template <typename T>
    struct raw_data_t
    {
        const T *data;
        size_t length;
    };

    template <typename T>
    class AppFeedBase
    {
    public:
        virtual ~AppFeedBase() = default;

        virtual void init() = 0;
        virtual const raw_data_t<T> *next() = 0;
    };

} // namespace app