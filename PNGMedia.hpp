#pragma once
#include "MediaType.hpp"
#include "crc32_impl/crc32.hpp"
#include <thread>
#include <unordered_map>
#include <cmath>
#include "zlib_impl/my_zlib_impl.hpp"
// я короче забыл что чанки идут так: длина чанка(не включая crc и тип чанка) -> тип чанка -> данные -> crc

namespace MyMediaTypes {
// PNG

    // PNG chucnks struct: IDHR -> 
    // cHRM/gAMA/iCCP xor sRGB/mDCV/cLLI/dBIT/eXLf/pHYs/sPLT/acTL -> 
    // bKGD/hIST/tRNS/eXLf/pHYs/sPLT/acTL -> PLTE(optional) ->
    // IDAT -> tIME/iTXt/tEXt/zTXt -> IEND

    class Pic_PNG : public Media_type {
        protected:
        std::vector<MediaTypePixel> palette;
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
            {"fcTL", true}, {"sPLT", true}, {"fdAT", true}, {"iTXt", true}, {"tEXt", true}, {"zTXt", true}
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
        
        // color type 0 - greyscale
        // color type 2 - truecolor
        // color type 3 - indexed
        // color type 4 - greyscale with alpha
        // color type 6 - truecolor with alpha
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
            if (details.color_type == 3 && (*chunks_visited_crit.find("PLTE")).second != true) {
                corrupted = true;
            }
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
                // adler-32 считается только для уже не сжатых распакованных данных
                // если fdict == 1, то 4 байта потом будет словарь который будет adler32
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
            if (idats == 0) {
                std::cout << "can't unpack zlib deflate without IDAT chunks\n";
                return;
            }
            auto not_filtered_data = unpack_zlib(idat_data);
            if (not_filtered_data == nullptr) {
                std::cout << "not filtered data is empty\n";
                return;
            }
            else {
                std::cout << "not filtered data is not empty\n";
                //filtering
                for (auto& d : *not_filtered_data) {
                    std::cout << (int)d << '\n';
                }
                delete not_filtered_data;
                return;
            }
        }

        void defilter_image(std::vector<uint8_t>* ref_data) {
            // для каждого канала идёт отдельно. то есть
            // должно выйти так что мы читаем фильтр
            // после прочтения мы получаем первую строку
            // если фильтр использует левый/левый верхний/верхний байты
            // то если мы находимся в 0, то считаем что левый/левый верхний/верхний байты = 0
            // ну и также мы должны для каждого отдельно всё это собирать, то есть у каждого будет свой
            // байт по которому он будет идти

            // грейскейл переводится как R = G = B = value, альфа канал там как пойдёт
            std::vector<uint8_t> r, g, b, a;
            auto& data = *ref_data;
            short r_offset, g_offset, b_offset, a_offset;
            // left = q - details.bit_on_pixel;
            // up = q - details.bit_on_pixel * width
            // up_left = up - details.bit_on_pixel
            switch (details.color_type)
            {
            case 0:
                r_offset = g_offset = b_offset = a_offset = 0;
            case 4:
                r_offset = g_offset = b_offset = 0;
                a_offset = 1;
                break;
            case 2:
                r_offset = 0;
                g_offset = 1;
                b_offset = 2;
                a_offset = 0;
                break;
            case 3:
                r_offset = 0;
                g_offset = 1;
                b_offset = 2;
                a_offset = 0;
                break;
            case 6:
                r_offset = 0;
                g_offset = 1;
                b_offset = 2;
                a_offset = 3;
                break;
            default:
                break;
            }
            for (std::size_t q = 0; q < data.size(); q += (details.bit_on_pixel/8) * width) {
                short filter = data[q];
                q++;
                for (std::size_t w = 0; w < width; w+= details.bit_on_pixel/8)
                switch (filter) {
                    case 0: // none, just copy
                        r.push_back(data[q+w+r_offset]);
                        g.push_back(data[q+w+g_offset]);
                        b.push_back(data[q+w+b_offset]);
                        if (details.color_type == 4 || details.color_type == 6) {
                            a.push_back(data[q+w+a_offset]);
                        }
                        else {
                            a.push_back(0);
                        }
                    break;
                    case 1: // sub left, byte now - byte left
                        if (w == 0) {
                            r.push_back(data[q+w+r_offset]);
                            g.push_back(data[q+w+g_offset]);
                            b.push_back(data[q+w+b_offset]);
                            if (details.color_type == 4 || details.color_type == 6) {
                                a.push_back(data[q+w+a_offset]);
                            }
                            else {
                                a.push_back(0);
                            }
                        }
                        else {
                            r.push_back(data[q+w+r_offset] - r[r.size() - 1]);
                            g.push_back(data[q+w+g_offset] - g[g.size() - 1]);
                            b.push_back(data[q+w+b_offset] - b[b.size() - 1]);
                            if (details.color_type == 4 || details.color_type == 6) {
                                a.push_back(data[q+w+a_offset] - a[a.size() - 1]);
                            }
                            else {
                                a.push_back(0);
                            }
                        }
                    break;
                    case 2: // sub up, byte now - byte up
                        r.push_back(data[q+w+r_offset] - r[r.size() - 1 - width]);
                        g.push_back(data[q+w+g_offset] - g[g.size() - 1 - width]);
                        b.push_back(data[q+w+b_offset] - b[b.size() - 1 - width]);
                        if (details.color_type == 4 || details.color_type == 6) {
                            a.push_back(data[q+w+a_offset] - a[a.size() - 1 - width]);
                        }
                        else {
                            a.push_back(0);
                        }
                    break;
                    case 3: // avg, byte now - (byte left + byte up) / 2
                        if (w == 0) {
                            r.push_back(data[q+w+r_offset] - r[r.size() - 1]);
                            g.push_back(data[q+w+g_offset] - g[g.size() - 1]);
                            b.push_back(data[q+w+b_offset] - b[b.size() - 1]);
                            if (details.color_type == 4 || details.color_type == 6) {
                                a.push_back(data[q+w+a_offset] - a[a.size() - 1]);
                            }
                            else {
                                a.push_back(0);
                            }
                        }
                        else {
                            r.push_back(data[q+w+r_offset] - (r[r.size() - 1] + r[r.size() - 1 - width]));
                            g.push_back(data[q+w+g_offset] - (g[g.size() - 1] + g[g.size() - 1 - width]));
                            b.push_back(data[q+w+b_offset] - (b[b.size() - 1] + b[b.size() - 1 - width]));
                            if (details.color_type == 4 || details.color_type == 6) {
                                a.push_back(data[q+w+a_offset] - (a[a.size() - 1] + a[a.size() - 1 - width]));
                            }
                            else {
                                a.push_back(0);
                            }
                        }
                    break;
                    case 4:
                    // need to find v = byte up + byte left - byte upper left
                    // then sub from v byte up, byte left, byte upper left and store
                    // check who is minimal and this is the sub, byte now - who minimal
                    if (w == 0) {
                            short upper_r = r[r.size() - 1 - width] - r[r.size() - 1], upper_g = g[g.size() - 1 - width] - g[g.size() - 1], 
                                  upper_b = b[b.size() - 1 - width] - b[b.size() - 1], upper_a = a[a.size() - 1 - width] - a[a.size() - 1];
                            r.push_back(data[q+w+r_offset] - upper_r);
                            g.push_back(data[q+w+g_offset] - upper_g);
                            b.push_back(data[q+w+b_offset] - upper_b);
                            if (details.color_type == 4 || details.color_type == 6) {
                                a.push_back(data[q+w+a_offset] - upper_a);
                            }
                            else {
                                a.push_back(0);
                            }
                        }
                        else {
                            short upper_r = r[r.size() - 1 - width], upper_g = g[g.size() - 1 - width], 
                                  upper_b = b[b.size() - 1 - width], upper_a = a[a.size() - 1 - width];
                            short upper_left_r = r[r.size() - 2 - width], upper_left_g = g[g.size() - 2 - width],
                                  upper_left_b = b[b.size() - 2 - width], upper_left_a = a[a.size() - 2 - width];
                            short left_r = r[r.size() - 1], left_g = g[g.size() - 1], 
                                  left_b = b[b.size() - 1], left_a = a[a.size() - 1];
                            uint16_t r_v = upper_r - upper_left_r + left_r,
                                    g_v = upper_g - upper_left_g + left_g,
                                    b_v = upper_b - upper_left_b + left_b,
                                    a_v = upper_a - upper_left_a + left_a;
                            r.push_back(data[q+w+r_offset] - (r[r.size() - 1] + r[r.size() - 1 - width]));
                            g.push_back(data[q+w+g_offset] - (g[g.size() - 1] + g[g.size() - 1 - width]));
                            b.push_back(data[q+w+b_offset] - (b[b.size() - 1] + b[b.size() - 1 - width]));
                            if (details.color_type == 4 || details.color_type == 6) {
                                a.push_back(data[q+w+a_offset] - (a[a.size() - 1] + a[a.size() - 1 - width]));
                            }
                            else {
                                a.push_back(0);
                            }
                        }
                    break;
                    default:
                    std::cout << "error, no filter type like this\n";
                    corrupted = true;
                    return;
                }   
            }
        }
    };
}