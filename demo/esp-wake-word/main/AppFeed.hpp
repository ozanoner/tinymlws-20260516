// AppFeed — streams the bundled offline PCM sample (data/hiesp.h) to the
// inference pipeline in model-sized chunks via next().

#pragma once

#include <cstdlib>
#include <cstring>
#include "esp_log.h"
#include "AppFeedBase.hpp"
#include "hiesp.h"

namespace app
{
    class AppFeed : public AppFeedBase<int16_t>
    {
    public:
        virtual ~AppFeed()
        {
            if (audio_buffer != nullptr)
            {
                free(audio_buffer);
            }
        }

        void init() override
        {
        }

        void init(size_t chunk_size)
        {
            audio_chunksize = chunk_size;
            audio_buffer = (int16_t *)malloc(audio_chunksize);
            if (audio_buffer == nullptr)
            {
                ESP_LOGE(TAG, "Failed to allocate audio buffer of size: %d", audio_chunksize);
                return;
            }
            buffer.data = audio_buffer;
            buffer.length = audio_chunksize / sizeof(int16_t);
        }

        const raw_data_t<int16_t> *next() override
        {
            if ((chunks + 1) * audio_chunksize <= sizeof(hiesp))
            {
                std::memcpy(audio_buffer, hiesp + chunks * audio_chunksize, audio_chunksize);
                ++chunks;
                return &buffer;
            }

            return nullptr; // No more data
        }

    private:
        static constexpr const char *TAG = "app_feed";

        int current_index = 0;
        size_t audio_chunksize = 0;
        int16_t *audio_buffer{nullptr};
        raw_data_t<int16_t> buffer{nullptr, 0};
        int chunks = 0;
    };
}