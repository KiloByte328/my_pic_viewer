#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>

namespace MyMediaTypes {
    class Media_type{
        protected:
        std::size_t type, width, height;
        std::size_t size;
        int bit_depth, compression_method, color_type, filter_method, interlace_method, bit_on_pixel;
        bool corrupted = false;
        //std::map <const char*, bool> chunks_visited;
        std::string data;
        public:
        virtual int get_type() const { return type; };
        virtual int get_width() const { return width; };
        virtual int get_height() const { return height; };
        virtual std::size_t get_size() const { return size; };
        virtual std::string get_data() const {return data; };
        virtual int get_depth() const { return bit_depth; };
        virtual int get_compression() const { return compression_method; };
        virtual int get_color_type() const { return color_type; };
        virtual int get_filter() const { return filter_method; };
        virtual int get_interlace() const { return interlace_method; };
        virtual int get_bop() const { return bit_on_pixel; };
        virtual bool parse() { return false; };
        virtual void add_data(std::string new_data) { data.append(new_data); };
        virtual int pars_char_to_int(const char* input_data, std::size_t size_of_data) { (void)input_data; (void)size_of_data; return 0; };
        virtual ~Media_type() = default;
        Media_type() { data.clear(); }
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

    class Invalid_Media : public Media_type {
        public:
        Invalid_Media() {type = -1; width = 0; height = 0; size = -1; data.clear(); bit_depth = 0;
            compression_method = 0; color_type= 0; filter_method = 0; interlace_method = 0; bit_on_pixel = 0;  };
    };

    // PNG

    // PNG chucnks struct: IDHR -> 
    // cHRM/gAMA/iCCP xor sRGB/mDCV/cLLI/dBIT/eXLf/pHYs/sPLT/acTL -> 
    // bKGD/hIST/tRNS/eXLf/pHYs/sPLT/acTL -> PLTE(optional) ->
    // IDAT -> tIME/iTXt/tEXt/zTXt -> IEND

    class Pic_PNG : public Media_type {
        // возможно придётся сделать проверку по CRC32
        // может без мапы обойдёмся
        // protected:
        // std::map<const char*, bool> chunks_visited = {
        // {"IHDR", false}, {"PLTE", false}, {"IEND", false}, {"cHRM", false},
        // {"gAMA", false}, {"cICP", false}, {"mDCV", false}, {"cLLI", false}, 
        // {"iCCP", false}, {"sBIT", false}, {"sRGB", false}, {"hIST", false},
        // {"bKGD", false}, {"pHYs", false}, {"tIME", false}, {"tRNS", false}, 
        // {"eXIf", false}, {"sPLT", false}, {"acTL", false} };
        public:
        Pic_PNG() {type = 1; width = 800; height = 600; size = -1; data.clear(); bit_depth = 8; color_type = 6; compression_method = 0; filter_method = 0; interlace_method = 0; bit_on_pixel = 24; };
        Pic_PNG(std::string new_data) { 
            type = 1; width = 800; height = 600; size = -1; bit_depth = 8; color_type = 6; compression_method = 0; filter_method = 0; interlace_method = 0; bit_on_pixel = 24;
            data.clear(); data.append(new_data); };
        
        bool type_and_depth_check() {
            if (color_type == 0 && (bit_depth == 1 || bit_depth == 2 || bit_depth == 4 || bit_depth == 8 || bit_depth == 16)) {
                bit_on_pixel = bit_depth;
                return true;
            }
            if (color_type == 2 && (bit_depth == 8 || bit_depth == 16)) {
                bit_on_pixel = 3 * bit_depth;
                return true;
            }
            if (color_type == 3 && (bit_depth == 1 || bit_depth == 2 || bit_depth == 4 || bit_depth == 8)) {
                bit_on_pixel = bit_depth;
                return true;
            }
            if (color_type == 4 && (bit_depth == 8 || bit_depth == 16)) {
                bit_on_pixel = 2 * bit_depth;
                return true;
            }
            if (color_type == 6 && (bit_depth == 8 || bit_depth == 16)) {
                bit_on_pixel = 4 * bit_depth;
                return true;
            }
            return false;
        };

        virtual int pars_char_to_int(const char* input_data, std::size_t size_of_data) override { 
            int val = 0;
            for (std::size_t i = 0; i < size_of_data; i++) {
                val = (val << 8) + (unsigned char)input_data[i];
            }
            return val;
        };

        bool header_check_repeat_only() {
        std::size_t wah = 0;
        size = data.find("IEND");
        if (size == data.npos)
            corrupted = true;
        wah = data.find("IHDR", 0) + 4;
        if (wah == data.npos) {
            corrupted = true;
            return corrupted;
        } // если ihdr не найден, то и работать с файлом не можем так как мы не знаем его параметры
        // ищем ещё один IHDR, если есть, то это плохой пнг
        if (data.find("IHDR", wah + 4) != data.npos) corrupted = true;
        // chunks_visited["IHDR"] = true;
        width = 0;
        height = 0;
        width = pars_char_to_int(data.substr(wah, 4).c_str(), 4);
        height = pars_char_to_int(data.substr(wah + 4, 4).c_str(), 4);
        bit_depth = data[wah + 8];
        color_type = data[wah + 9];
        compression_method = data[wah + 10];
        filter_method = data[wah + 11];
        interlace_method = data[wah + 12];
        type_and_depth_check() == false ? corrupted = true : 0;
        return corrupted;
        };

        //checking checksum CRC32
        bool check_checksum() {
            return true;
        };

        std::size_t header_check_repeat_only(const char* header_name) {
            if (data.find(header_name, (data.find(header_name) + 4)) == data.npos)
                return data.find(header_name); 
            else 
                return (std::size_t)-1;
        }

        bool chunks_check() {
            //сделать проверки на противоречия в sRGB/iCCP
            //3 -> 10
            //can't be multiple in one file
            std::size_t wah = 0;
            std::size_t first_idat = data.find("IDAT");
            header_check_repeat_only("cHRM");
            header_check_repeat_only("cICP");
            header_check_repeat_only("gAMA");
            header_check_repeat_only("tIME");
            header_check_repeat_only("eXIf");
            wah = data.find("iCCP");
            if (wah != data.npos) {
                if (data.find("iCCP", wah + 4) != data.npos || 
                    data.find("sRGB") != data.npos)
                corrupted = true;
            }
            wah = data.find("mDCV");
            if (wah != data.npos) {
                if (data.find("mDCV", wah + 4) != data.npos)
                corrupted = true;
            }
            wah = data.find("cLLI");
            if (wah != data.npos) {
                if (data.find("cLLI", wah + 4) != data.npos)
                corrupted = true;
            }
            wah = data.find("sBIT");
            if (wah != data.npos) {
                if (data.find("sBIT", wah + 4) != data.npos)
                corrupted = true;
            }
            wah = data.find("sRGB");
            if (wah != data.npos) {
                if (data.find("sRGB", wah + 4) != data.npos ||
                    data.find("iCCP") != data.npos)
                corrupted = true;
            }
            wah = data.find("pHYs");
            if (wah != data.npos) {
                if (data.find("pHYs", wah + 4) != data.npos)
                corrupted = true;
            }
            wah = data.find("PLTE");
            if (wah != data.npos) {
                if (data.find("PLTE", wah + 4) != data.npos ||
                    data.find("IDAT") < wah)
                    corrupted = true;
            }
            return corrupted;
        };

        //return true if file is corrupted else false
        virtual bool parse() override {
            header_check_repeat_only();
            if (corrupted) return corrupted;
            chunks_check();
            if (corrupted) return corrupted;

            return corrupted;
        };
    };

    // JPEG
    class Pic_JPEG : public Media_type {
        public:
        Pic_JPEG() {
            type = 2;
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