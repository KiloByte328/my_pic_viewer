#pragma once
#include "MediaType.hpp"
#include "crc32_impl/crc32.hpp"
#include <thread>
#include <unordered_map>
#include <cmath>
// я короче забыл что чанки идут так: длина чанка(не включая crc и тип чанка) -> тип чанка -> данные -> crc

namespace MyMediaTypes {
// PNG

    // PNG chucnks struct: IDHR -> 
    // cHRM/gAMA/iCCP xor sRGB/mDCV/cLLI/dBIT/eXLf/pHYs/sPLT/acTL -> 
    // bKGD/hIST/tRNS/eXLf/pHYs/sPLT/acTL -> PLTE(optional) ->
    // IDAT -> tIME/iTXt/tEXt/zTXt -> IEND

    class Pic_PNG : public Media_type {
        protected:
        std::map<std::string, bool> chunks_visited_crit = {
            {"IHDR", false}, {"PLTE", false}, {"IDAT", false}, {"IEND", false}
        };
        std::map<std::string, bool> chunks_visited_secondary = { 
            {"cHRM", false}, {"fcTL", false}, {"tIME", false}, {"tEXt", false},
            {"gAMA", false}, {"cICP", false}, {"mDCV", false}, {"cLLI", false}, 
            {"iCCP", false}, {"sBIT", false}, {"sRGB", false}, {"hIST", false},
            {"bKGD", false}, {"pHYs", false}, {"tIME", false}, {"tRNS", false}, 
            {"eXIf", false}, {"sPLT", false}, {"fdAT", false}, {"iTXt", false},
            {"acTL", false}
        };
        std::unordered_map<std::string, bool> multy= {
            {"fcTL", true}, {"sPLT", true}, {"fdAT", 1}, {"iTXt", 1}, {"tEXt", 1}, {"zTXt", 1}
        };
        public:
        Pic_PNG() {
                width = 800; height = 600; data.clear(); size = -1;
                details = {
                .bit_depth = 8, .compression_method = 0, 
                .color_type = 6, .filter_method = 0, 
                .interlace_method = 0, .bit_on_pixel = 2, .type = 1
                };
            }
        Pic_PNG(std::string new_data) { 
            width = 800; height = 600; size = -1; 
            details = {
                .bit_depth = 8, .compression_method = 0, 
                .color_type = 6, .filter_method = 0, 
                .interlace_method = 0, .bit_on_pixel = 2, .type = 1
            };
            data.clear(); data.append(new_data); }
        
        bool type_and_depth_check() {
            if (details.color_type == 0 && (details.bit_depth == 1 || details.bit_depth == 2 || details.bit_depth == 4 || details.bit_depth == 8 || details.bit_depth == 16)) {
                details.bit_on_pixel = details.bit_depth;
                return true;
            }
            if (details.color_type == 2 && (details.bit_depth == 8 || details.bit_depth == 16)) {
                details.bit_on_pixel = 3 * details.bit_depth;
                return true;
            }
            if (details.color_type == 3 && (details.bit_depth == 1 || details.bit_depth == 2 || details.bit_depth == 4 || details.bit_depth == 8)) {
                details.bit_on_pixel = details.bit_depth;
                return true;
            }
            if (details.color_type == 4 && (details.bit_depth == 8 || details.bit_depth == 16)) {
                details.bit_on_pixel = 2 * details.bit_depth;
                return true;
            }
            if (details.color_type == 6 && (details.bit_depth == 8 || details.bit_depth == 16)) {
                details.bit_on_pixel = 4 * details.bit_depth;
                return true;
            }
            return false;
        }

        virtual long pars_char_to_int(const char* input_data, std::size_t size_of_data) override { 
            long val = 0;
            for (std::size_t i = 0; i < size_of_data; i++) {
                val = (val << 8) + (unsigned char)input_data[i];
            }
            return val;
        }

        bool header_check() {
            std::size_t wah = 8;
            while(true) {
                std::size_t next_chunk = pars_char_to_int(data.substr(wah, 4).c_str(), 4);
                std::string chunk_header = data.substr(wah+4, 4);
                if (chunk_header.compare("IHDR") == 0) {
                    width = 0;
                    height = 0;
                    width = pars_char_to_int(data.substr(wah + 8, 4).c_str(), 4);
                    height = pars_char_to_int(data.substr(wah + 12, 4).c_str(), 4);
                    details.bit_depth = data[wah + 16];
                    details.color_type = data[wah + 17];
                    details.compression_method = data[wah + 18];
                    details.filter_method = data[wah + 19];
                    details.interlace_method = data[wah + 20];
                    type_and_depth_check() == false ? corrupted = true : 0;
                }
                if (chunks_visited_crit.contains(chunk_header)) {
                    if (chunk_header.compare("IDAT") != 0)
                        if (chunks_visited_crit[chunk_header] == true) {
                            corrupted = true;
                            return corrupted;
                        }
                    chunks_visited_crit[chunk_header] = true;
                }
                if (chunks_visited_secondary.contains(chunk_header)) {
                    if (!multy.contains(chunk_header))
                        if (chunks_visited_secondary[chunk_header] == true) {
                            corrupted = true;
                        }
                    chunks_visited_secondary[chunk_header] = true;
                }
                if (check_checksum(wah+4) != true) {
                    std::cout << "error in crc32 at chunk " << data.substr(wah+4, 4) << '\n';
                }
                if (chunk_header.compare("IEND") == 0)
                    break;
                wah += next_chunk + 12;
            }
            size = wah + 12;
            return corrupted;
        }

        bool check_checksum(std::size_t id) {
            std::size_t cur_size = pars_char_to_int(data.substr(id-4, 4).c_str(), 4)+4;
            std::size_t check_sm = id + cur_size;
            auto check_crc = data.substr(id, cur_size);
            unsigned long our_crc = pars_char_to_int(data.substr(check_sm).c_str(), 4);
            auto q = crc((unsigned char*)check_crc.c_str(), cur_size);
            return q == our_crc ? true : false;
        }

        //return true if file is corrupted else false
        virtual bool parse() override {
            header_check();
            if (corrupted) return corrupted;
            decode_image();
            return false;
        }

        virtual void decode_image() override {
            std::size_t wah = 8;
            std::string idat_data, dict;
            int method;
            unsigned int window;
            short flv;
            int fcheck;
            int fdict;
            int idats = 0;
            while (true) {
                std::size_t next_chunk = pars_char_to_int(data.substr(wah, 4).c_str(), 4);
                std::string chunk_header = data.substr(wah+4, 4);
                //adler-32 считается только для уже не сжатых распакованных данных
                if (chunk_header.compare("IDAT") == 0) {
                    if (idats == 0) {
                        method = data[wah+8] & 0xf;
                        window = pow(2, (data[wah+8]>> 4) + 8);
                        idats++;
                        flv = (data[wah+9] & 0xc0) >> 6;
                        fdict = (data[wah+9] & 0x20) >> 5;
                        fcheck = data[wah+9] & 0x1f;
                        details.comp_lv = flv;
                        details.window = window;
                        details.dict = fdict;
                        if (fcheck != 0)
                            if (data[wah+8]*256 + data[wah+9] / 31 != 0)
                                corrupted = true;
                    }
                    idat_data.append(data.substr(wah+8, next_chunk));
                }
                if (chunk_header.compare("IEND") == 0) {
                    break;
                }
                wah += next_chunk + 12;
            }
            // std::cout << idat_data << '\n';
            std::cout << method << '\n' << window << '\n' << flv << '\n' << fdict <<'\n';
        }
    };
}