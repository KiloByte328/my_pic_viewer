#pragma once
#include "MediaType.hpp"

namespace MyMediaTypes {
    // JPEG
    class Pic_JPEG : public Media_type {
        public:
        Pic_JPEG() {
            width = 800;
            height = 600;
            size = -1;
            data.clear();
            details =
            {
                .bit_depth = 8,
                .compression_method = 1,
                .color_type = 3,                
                .filter_method = 0,
                .interlace_method = 0,
                .bit_on_pixel = 24,
                .type = 2,
            };
        }
    };

}