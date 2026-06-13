#pragma once
#include "MyMediaTypes.hpp"
#include <memory>
#include <string>

namespace MyMediaTypes {
    class MediaFactory {
    public:
        static std::unique_ptr<Media_type> CreateImage(std::size_t width, std::size_t height) {
            auto image = std::make_unique<Pic_PNG>(width, height);
            return image;
        }

        static std::unique_ptr<Media_type> LoadImage(const std::string& path) {
            std::fstream fl;
            fl.open(path, std::ios::in);
            if (!fl.is_open()) {
                std::cout << "cant open file\n";
                fl.close();
                return std::make_unique<Invalid_Media>();
            }
            std::stringstream buf;
            buf << fl.rdbuf();
            return CreateImageFromData(buf.str());
            fl.close();
        }

        static std::unique_ptr<Media_type> ParseImage(int type) {
            switch (type) {
                case 1: // PNG
                    return std::make_unique<Pic_PNG>();
                case 2: // JPEG
                    return std::make_unique<Pic_JPEG>();
                case 3: // WEBP
                    return std::make_unique<Pic_WEBP>();
                case 4: // QOI
                    return std::make_unique<Pic_QOI>();
                default:
                    return std::make_unique<Invalid_Media>();
            }
        }

        static std::unique_ptr<Media_type> CreateImageFromData(const std::string& data) {
            if (data.empty()) {
                return std::make_unique<Invalid_Media>();
            }

            if (data.size() >= 8) {
                // PNG signature check
                if (data.substr(0, 5).compare("\211PNG\r") == 0) {
                    auto png = std::make_unique<Pic_PNG>(data);
                    if (!png->parse()) {
                        return png;
                    }
                    return std::make_unique<Invalid_Media>();
                }
                // QOI signature check
                else if (data.substr(0, 4).compare("qoif") == 0) {
                    auto qoi = std::make_unique<Pic_QOI>();
                    qoi->add_data(data);
                    if (qoi->parse()) {
                        return qoi;
                    }
                    return std::make_unique<Invalid_Media>();
                }
                // WEBP signature check (RIFF header)
                else if (data.substr(0, 4).compare("RIFF") == 0) {
                    return std::make_unique<Pic_WEBP>();
                }
                // JPEG signature check
                else if (data[0] == 0xFF && data[1] == 0xD8) {
                    return std::make_unique<Pic_JPEG>();
                }
            }

            return std::make_unique<Invalid_Media>();
        }
    };
}
