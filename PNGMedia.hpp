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
                // for (auto& d : *not_filtered_data) {
                //     std::cout << (int)d << '\n';
                // }
                defilter_image(not_filtered_data);
                delete not_filtered_data;
                return;
            }
        }

        void defilter_image(std::vector<uint8_t>* ref_data) {
            if (ref_data == nullptr || ref_data->empty()) return;
            
            // Очищаем векторы для хранения распакованных данных
            std::vector<uint8_t> r, g, b, a;
            
            auto& data = *ref_data;
            
            // Определяем смещения для каналов
            short r_offset, g_offset, b_offset, a_offset;
            short bytes_per_pixel;
            
            switch (details.color_type) {
                case 0: // Grayscale
                    r_offset = g_offset = b_offset = a_offset = 0;
                    bytes_per_pixel = details.bit_depth / 8;
                    break;
                case 4: // Grayscale with alpha
                    r_offset = g_offset = b_offset = 0;
                    a_offset = 1;
                    bytes_per_pixel = details.bit_depth / 4; // 2 bytes per pixel
                    break;
                case 2: // RGB
                    r_offset = 0;
                    g_offset = 1;
                    b_offset = 2;
                    a_offset = 0;
                    bytes_per_pixel = details.bit_depth / 8 * 3;
                    break;
                case 3: // Palette
                    r_offset = 0;
                    g_offset = 1;
                    b_offset = 2;
                    a_offset = 0;
                    bytes_per_pixel = 1;
                    break;
                case 6: // RGBA
                    r_offset = 0;
                    g_offset = 1;
                    b_offset = 2;
                    a_offset = 3;
                    bytes_per_pixel = details.bit_depth / 8 * 4;
                    break;
                default:
                    std::cout << "Unsupported color type: " << details.color_type << std::endl;
                    corrupted = true;
                    return;
            }
            
            // Обрабатываем каждую строку
            std::size_t row = 0;
            while (row < data.size()) {
                if (row >= data.size()) break;
                
                // Читаем тип фильтра (1 байт в начале строки)
                if (row + 1 > data.size()) break;
                
                short filter = data[row];
                row++; // Пропускаем фильтр
                
                // Обрабатываем пиксели в строке
                for (std::size_t pixel = 0; pixel < width; ++pixel) {
                    if (row >= data.size()) break;
                    
                    // Получаем позицию пикселя
                    std::size_t byte_pos = row + pixel * bytes_per_pixel;
                    if (byte_pos + bytes_per_pixel > data.size()) break;
                    
                    switch (filter) {
                        case 0: // None
                            for (int i = 0; i < bytes_per_pixel; i++) {
                                if (byte_pos + i < data.size()) {
                                    if (i == r_offset) r.push_back(data[byte_pos + i]);
                                    if (i == g_offset) g.push_back(data[byte_pos + i]);
                                    if (i == b_offset) b.push_back(data[byte_pos + i]);
                                    if (i == a_offset) a.push_back(data[byte_pos + i]);
                                }
                            }
                            break;
                            
                        case 1: // Sub
                            for (int i = 0; i < bytes_per_pixel; i++) {
                                if (byte_pos + i < data.size()) {
                                    uint8_t value = data[byte_pos + i];
                                    if (pixel > 0) {
                                        // Получаем значение предыдущего пикселя
                                        uint8_t prev_value = 0;
                                        if (i < r.size()) prev_value = r[r.size() - bytes_per_pixel + i];
                                        else if (i < g.size()) prev_value = g[g.size() - bytes_per_pixel + i];
                                        else if (i < b.size()) prev_value = b[b.size() - bytes_per_pixel + i];
                                        else if (i < a.size()) prev_value = a[a.size() - bytes_per_pixel + i];
                                        value += prev_value;
                                    }
                                    if (i == r_offset) r.push_back(value);
                                    if (i == g_offset) g.push_back(value);
                                    if (i == b_offset) b.push_back(value);
                                    if (i == a_offset) a.push_back(value);
                                }
                            }
                            break;
                            
                        case 2: // Up
                            for (int i = 0; i < bytes_per_pixel; i++) {
                                if (byte_pos + i < data.size()) {
                                    uint8_t value = data[byte_pos + i];
                                    if (row > bytes_per_pixel) {
                                        // Получаем значение пикселя сверху
                                        std::size_t up_pos = row - bytes_per_pixel + i;
                                        if (up_pos < data.size()) {
                                            value += data[up_pos];
                                        }
                                    }
                                    if (i == r_offset) r.push_back(value);
                                    if (i == g_offset) g.push_back(value);
                                    if (i == b_offset) b.push_back(value);
                                    if (i == a_offset) a.push_back(value);
                                }
                            }
                            break;
                            
                        case 3: // Average
                            for (int i = 0; i < bytes_per_pixel; i++) {
                                if (byte_pos + i < data.size()) {
                                    uint8_t value = data[byte_pos + i];
                                    if (pixel > 0) {
                                        // Среднее от левого и верхнего пикселя
                                        uint8_t left_value = 0;
                                        uint8_t up_value = 0;
                                        
                                        if (r.size() > 0 && i < r.size()) {
                                            left_value = r[r.size() - bytes_per_pixel + i];
                                        }
                                        if (row > bytes_per_pixel && i < data.size()) {
                                            std::size_t up_pos = row - bytes_per_pixel + i;
                                            if (up_pos < data.size()) {
                                                up_value = data[up_pos];
                                            }
                                        }
                                        value += (left_value + up_value) / 2;
                                    }
                                    if (i == r_offset) r.push_back(value);
                                    if (i == g_offset) g.push_back(value);
                                    if (i == b_offset) b.push_back(value);
                                    if (i == a_offset) a.push_back(value);
                                }
                            }
                            break;
                            
                        case 4: // Paeth
                            for (int i = 0; i < bytes_per_pixel; i++) {
                                if (byte_pos + i < data.size()) {
                                    uint8_t value = data[byte_pos + i];
                                    if (pixel > 0) {
                                        // Вычисляем предиктор Paeth
                                        uint8_t left_value = 0;
                                        uint8_t up_value = 0;
                                        uint8_t up_left_value = 0;
                                        
                                        if (r.size() > 0 && i < r.size()) {
                                            left_value = r[r.size() - bytes_per_pixel + i];
                                        }
                                        if (row > bytes_per_pixel && i < data.size()) {
                                            std::size_t up_pos = row - bytes_per_pixel + i;
                                            if (up_pos < data.size()) {
                                                up_value = data[up_pos];
                                            }
                                        }
                                        if (row > bytes_per_pixel && r.size() > bytes_per_pixel && i < r.size()) {
                                            std::size_t up_left_pos = row - bytes_per_pixel + i - bytes_per_pixel;
                                            if (up_left_pos < data.size()) {
                                                up_left_value = data[up_left_pos];
                                            }
                                        }
                                        
                                        // Простой предиктор
                                        int pred = left_value + up_value - up_left_value;
                                        int diff_left = abs(pred - left_value);
                                        int diff_up = abs(pred - up_value);
                                        int diff_up_left = abs(pred - up_left_value);
                                        
                                        int result = pred;
                                        if (diff_left <= diff_up && diff_left <= diff_up_left) {
                                            result = left_value;
                                        } else if (diff_up <= diff_up_left) {
                                            result = up_value;
                                        } else {
                                            result = up_left_value;
                                        }
                                        
                                        value += result;
                                    }
                                    if (i == r_offset) r.push_back(value);
                                    if (i == g_offset) g.push_back(value);
                                    if (i == b_offset) b.push_back(value);
                                    if (i == a_offset) a.push_back(value);
                                }
                            }
                            break;
                            
                        default:
                            std::cout << "Unsupported filter type: " << filter << std::endl;
                            corrupted = true;
                            return;
                    }
                }
                // Переход к следующей строке
                row += bytes_per_pixel * width;
            }
            
            // Переписываем данные обратно в ref_data
            data.clear();
            for (size_t i = 0; i < r.size(); ++i) {
                data.push_back(r[i]);
                data.push_back(g[i]);
                data.push_back(b[i]);
                std::cout << r[i] << " " << g[i] << " " << b[i] << ' ';
                if (details.color_type == 4 || details.color_type == 6) {
                    data.push_back(a[i]);
                    std::cout << a[i];
                }
                std::cout << '\n';
            }
        }
    };
}