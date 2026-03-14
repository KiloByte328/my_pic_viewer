#pragma once
#include "MediaType.hpp"
#include "crc32_impl/crc32.hpp"
#include <thread>
// я короче забыл что чанки идут так: длина чанка(не включая crc и тип чанка) -> тип чанка -> данные -> crc

namespace MyMediaTypes {
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
            std::size_t wah = 0;
            size = data.find("IEND") + 8;
            if (size == data.npos)
                return corrupted = true;
            if (check_checksum(data.find("IEND")) != true) {
                std::cerr << "wrong checksum in IEND\n";
                corrupted = true;
            }
            else {
                std::cerr << "checksum verified\n";
            }
            wah = data.find("IHDR", 0) + 4;
            if (wah == data.npos) {
                corrupted = true;
                return corrupted;
            }
            if (check_checksum(data.find("IHDR")) != true) {
                std::cerr << "wrong checksum in IHDR\n";
                corrupted = true;
            }
            else {
                std::cerr << "checksum IHDR verified\n";
            }
            width = 0;
            height = 0;
            width = pars_char_to_int(data.substr(wah, 4).c_str(), 4);
            height = pars_char_to_int(data.substr(wah + 4, 4).c_str(), 4);
            details.bit_depth = data[wah + 8];
            details.color_type = data[wah + 9];
            details.compression_method = data[wah + 10];
            details.filter_method = data[wah + 11];
            details.interlace_method = data[wah + 12];
            type_and_depth_check() == false ? corrupted = true : 0;
            wah = data.find("PLTE");
            if (wah != data.npos) {
                if (data.find("PLTE", wah + 4) != data.npos || 
                    data.find("IDAT") < wah)
                    corrupted = true;
            }
            return corrupted;
        }

        bool check_checksum(std::size_t id) {
            std::size_t cur_size = pars_char_to_int(data.substr(id-4, 4).c_str(), 4)+4;
            std::size_t check_sm = id + cur_size;
            std::cout << cur_size << '\n' << check_sm <<'\n';
            auto check_crc = data.substr(id, cur_size);
            std::cout << data.substr(id, cur_size) << '\n';
            unsigned long our_crc = pars_char_to_int(data.substr(check_sm).c_str(), 4);
            auto q = crc((unsigned char*)check_crc.c_str(), cur_size);
            std::cout << check_crc << '\n' << q << '\n' << our_crc << '\n';
            return q == our_crc ? true : false;
        }

        std::size_t chunks_check(const char* header_name) {
            if (data.find(header_name) != (std::size_t)-1) {
                if (check_checksum(data.find(header_name)) != true) {
                    std::cerr << "chunk " << header_name << " has wrong checksum\n";
                }
                else {
                    std::cerr << "chunk " << header_name << " verified checksum\n";
                }
            }
            if (data.find(header_name, (data.find(header_name) + 4)) == data.npos)
                return data.find(header_name);
            else 
                return (std::size_t)-1;
        }

        bool chunks_check() {
            //сделать проверки на противоречия в sRGB/iCCP
            //3 -> 10
            //can't be multiple in one file
            // unsigned int threads = std::thread::hardware_concurrency();
            // if (threads != 0) {
            // возможно раскинуть проверку чанков на несколько потоков, мб сделать свою проверку на количество потоков
            // }
            std::size_t wah = 0;
            std::size_t first_idat = data.find("IDAT");
            chunks_check("cHRM");
            chunks_check("cICP");
            chunks_check("gAMA");
            chunks_check("tIME");
            chunks_check("eXIf");
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
        }

        //return true if file is corrupted else false
        virtual bool parse() override {
            header_check();
            if (corrupted) return corrupted;
            chunks_check();
            if (corrupted) return corrupted;

            return corrupted;
        }
    };
}