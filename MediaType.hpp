#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <cstdint>
#include <vector>

// для работы в qt будем приводить к 
// const QRgb rgb1 = 0x88112233;
// const QRgb rgb2 = QColor("red").rgb();
// const QRgb rgb3 = qRgb(qRed(rgb1), qGreen(rgb2), qBlue(rgb2));
// const QRgb rgb4 = qRgba(qRed(rgb1), qGreen(rgb2), qBlue(rgb2), qAlpha(rgb1));

namespace MyMediaTypes {
    struct MediaTypeImageDetails
    {
        int bit_depth, 
        compression_method, 
        color_type, 
        filter_method, 
        interlace_method, 
        bit_on_pixel, 
        type, 
        window = 0, 
        cm = 0,
        dict = 0, 
        comp_lv = 0;
    };
    
    struct MediaTypePixels
    {
        uint8_t r, g, b, a;
    };
    
    class ImageType {
        MediaTypeImageDetails my_details;
        std::vector<MediaTypePixels> Pixels;
        std::size_t width, height;
    };

    class Media_type{
        protected:
        std::size_t width, height;
        std::size_t size;
        MediaTypeImageDetails details;
        bool corrupted = false;
        std::vector<MediaTypePixels> pixels_data;
        //std::map <const char*, bool> chunks_visited;
        std::string data;
        public:
        virtual void decode_image() {}
        virtual int get_type() const { return details.type; }
        virtual std::size_t get_width() const { return width; }
        virtual std::size_t get_height() const { return height; }
        virtual std::size_t get_size() const { return size; }
        virtual std::string get_data() const {return data; }
        virtual int get_depth() const { return details.bit_depth; }
        virtual int get_compression() const { return details.compression_method; }
        virtual int get_color_type() const { return details.color_type; }
        virtual int get_filter() const { return details.filter_method; }
        virtual int get_interlace() const { return details.interlace_method; }
        virtual int get_bop() const { return details.bit_on_pixel; }
        virtual bool parse() { return false; }
        virtual void add_data(std::string new_data) { data.append(new_data); }
        virtual long pars_char_to_int(const char* input_data, std::size_t size_of_data) { (void)input_data; (void)size_of_data; return 0; }
        virtual ~Media_type() = default;
        Media_type() { data.clear(); }
    };
    class Invalid_Media : public Media_type {
        public:
        Invalid_Media() {width = 0; height = 0; size = -1; data.clear(); details =
            {
                .bit_depth = 8,
                .compression_method = 1,
                .color_type = 3,                
                .filter_method = 0,
                .interlace_method = 0,
                .bit_on_pixel = 24,
                .type = -1,
            };  
        }
    };
    std::ostream& operator <<(std::ostream& os, const Media_type& mt) {
        os << "type: ";
        switch (mt.get_type()) {
            case 1:
                os << "PNG";
            break;
            case 2:
                os << "JPEG";
            break;
            case 3:
                os << "WEBP";
            break;
            default: 
                os << "invalid type";
            break;
        }
        os << "\nwidth: " << mt.get_width() << "\nheigth: " << mt.get_height() 
        << "\nsize: " << mt.get_size() << "\nbits depth: " << mt.get_depth() << "\ncolor_type: " << mt.get_color_type() << "\nbits on pixel: " << mt.get_bop()
        << "\nfilter: " << mt.get_filter() << "\nintelance: " << mt.get_interlace() << "\ncompression: " << mt.get_compression();
        return os;
    }
}