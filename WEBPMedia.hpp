#pragma once
#include "MediaType.hpp"
namespace MyMediaTypes {
    // RIFF *File size* WEBP
    class Pic_WEBP : public Media_type {
        protected:
        int color_type;
        public:
        Pic_WEBP() {
            type = 3;
            width = 800;
            height = 600;
            size = -1;
            data.clear();
            bit_depth = 8;
            color_type = 3;
            compression_method = 1;
            filter_method = 0;
            interlace_method = 0;
            bit_on_pixel = 24;
        };
    };
}