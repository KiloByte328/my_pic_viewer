#pragma once
#include "MediaType.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>

// Предположим, что у нас уже есть namespace MyMediaTypes из предыдущего кода

namespace MyMediaTypes {

    // Заголовок QOI (8 байт)
    struct QOI_HEADER {
        uint32_t magic;       // 4 байта: "qoif"
        uint32_t width;       // 4 байта: ширина изображения
        uint32_t height;      // 4 байта: высота изображения
        uint8_t channels;     // 1 байт: количество каналов (3 или 4)
        uint8_t colorspace;   // 1 байт: 0 = sRGB with linear alpha, 1 = all channels linear
    };

    // Класс QOI-изображения
    class Pic_QOI : public Media_type {
    public:
        Pic_QOI() {
            details = {
                .bit_depth = 8,
                .compression_method = 0,
                .color_type = 3,
                .filter_method = 0,
                .interlace_method = 0,
                .bit_on_pixel = 32,
                .type = 4, // QOI
            };
        }

        bool parse() override {
            if (data.empty()) {
                return false;
            }

            if (data.size() < sizeof(QOI_HEADER)) {
                corrupted = true;
                return false;
            }

            QOI_HEADER header;
            std::memcpy(&header, data.data(), sizeof(QOI_HEADER));

            // Проверка магического числа
            if (header.magic != 0x716F6966) {
                corrupted = true;
                return false;
            }

            width = header.width;
            height = header.height;

            // Проверка корректности размеров (ограничение по стандарту)
            if (width == 0 || height == 0 || width > 0x3FFF || height > 0x3FFF) {
                corrupted = true;
                return false;
            }

            decode_image();
            return !corrupted;
        }

        void decode_image() override {
            if (data.empty()) return;

            std::vector<uint8_t> bytes(data.begin(), data.end());
            size_t pos = sizeof(QOI_HEADER);

            pixels_data.clear();
            pixels_data.reserve(width * height);

            std::vector<MediaTypePixel> pixel_buffer(width * height);
            MediaTypePixel previous = {0, 0, 0, 255};

            size_t i = 0;
            while (pos < bytes.size() && i < width * height) {
                uint8_t byte = bytes[pos++];

                if (byte == 0x00) { // QOI_OP_INDEX
                    uint8_t index = bytes[pos++];
                    previous = pixel_buffer[index];
                }
                else if ((byte & 0xC0) == 0x00) { // QOI_OP_RGB
                    previous.r = bytes[pos++];
                    previous.g = bytes[pos++];
                    previous.b = bytes[pos++];
                    previous.a = 255;
                }
                else if ((byte & 0xC0) == 0x40) { // QOI_OP_RGBA
                    previous.r = bytes[pos++];
                    previous.g = bytes[pos++];
                    previous.b = bytes[pos++];
                    previous.a = bytes[pos++];
                }
                else if ((byte & 0xC0) == 0x80) { // QOI_OP_RUN
                    int run = (byte & 0x3F);
                    for (int j = 0; j <= run; ++j) {
                        pixel_buffer[i++] = previous;
                    }
                    continue;
                }
                else if ((byte & 0xC0) == 0xC0) { // QOI_OP_DIFF
                    int dr = ((byte >> 4) & 0x03) - 2;
                    int dg = ((byte >> 2) & 0x03) - 2;
                    int db = (byte & 0x03) - 2;
                    previous.r = (previous.r + dr) & 0xFF;
                    previous.g = (previous.g + dg) & 0xFF;
                    previous.b = (previous.b + db) & 0xFF;
                    previous.a = 255;
                }
                else if ((byte & 0xC0) == 0x00) { // QOI_OP_RGB
                    previous.r = bytes[pos++];
                    previous.g = bytes[pos++];
                    previous.b = bytes[pos++];
                }
                else if ((byte & 0xC0) == 0x40) { // QOI_OP_RGBA
                    previous.r = bytes[pos++];
                    previous.g = bytes[pos++];
                    previous.b = bytes[pos++];
                    previous.a = bytes[pos++];
                }
                else {
                    // Ошибка или неизвестная операция
                    corrupted = true;
                    return;
                }

                pixel_buffer[i++] = previous;
            }

            pixels_data = pixel_buffer;
        }

        std::size_t get_width() const override { return width; }
        std::size_t get_height() const override { return height; }
        std::vector<MediaTypePixel> get_pixels() const { return pixels_data; }

        static bool load_file(const std::string& filename, Pic_QOI& img) {
            std::ifstream file(filename, std::ios::binary | std::ios::ate);
            if (!file.is_open()) {
                return false;
            }

            std::size_t size = file.tellg();
            file.seekg(0, std::ios::beg);

            std::vector<char> buffer(size);
            file.read(buffer.data(), size);
            file.close();

            img.add_data(std::string(buffer.begin(), buffer.end()));
            return img.parse();
        }
    };
}
