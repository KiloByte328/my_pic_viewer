#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>

class Media_type{
    protected:
    std::size_t type, width, height;
    std::size_t size;
    std::string data;
    int bit_depth, compression_method, color_type, filter_method, interlace_method, bit_on_pixel;
    std::map <const char*, bool> chunks_visited;
    bool corrupted = false;
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

class invalid_type : public Media_type {
    public:
    invalid_type() {type = -1; width = 0; height = 0; size = -1; data.clear(); };
};

// PNG

// PNG chucnks struct: IDHR -> 
// cHRM/gAMA/iCCP xor sRGB/mDCV/cLLI/dBIT/eXLf/pHYs/sPLT/acTL -> 
// bKGD/hIST/tRNS/eXLf/pHYs/sPLT/acTL -> PLTE(optional) ->
// IDAT -> tIME/iTXt/tEXt/zTXt -> IEND

class pic_png : public Media_type {
    protected:
    std::map<const char*, bool> chunks_visited = {
    {"IHDR", false}, {"PLTE", false}, {"IEND", false}, {"cHRM", false},
    {"gAMA", false}, {"cICP", false}, {"mDCV", false}, {"hIST", false},
    {"cLLI", false}, {"iCCP", false}, {"sBIT", false}, {"sRGB", false},
    {"bKGD", false}, {"pHYs", false}, {"tIME", false}, {"tRNS", false}, 
    {"eXIf", false}, {"sPLT", false}, {"acTL", false} };
    public:
    pic_png() {type = 1; width = 800; height = 600; size = -1; data.clear(); bit_depth = 8; color_type = 6; compression_method = 0; filter_method = 0; interlace_method = 0; bit_on_pixel = 24; };
    pic_png(std::string new_data) { 
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

    bool header_check() {
    std::size_t wah = 0;
    size = data.find("IEND", 0);
    if (size == data.npos)
        corrupted = true;
    wah = data.find("IHDR", 0) + 4; //directly going to information of IHDR
    chunks_visited["IHDR"] = true;
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

    virtual bool parse() override {
        std::size_t wah = 0;
        header_check() ? corrupted = true : 0;
        if (corrupted) return corrupted;

    };
};

// JPEG
class pic_jpeg : public Media_type {
    public:
    pic_jpeg() {type = 2; width = 800; height = 600; size = -1; data.clear(); bit_depth = 8; color_type = 3; compression_method = 1; filter_method = 0; interlace_method = 0; bit_on_pixel = 24; };
};

// RIFF *File size* WEBP
class pic_webp : public Media_type {
    protected:
    int color_type;
    public:
    pic_webp() {type = 3; width = 800; height = 600; size = -1; data.clear(); bit_depth = 8; color_type = 3; compression_method = 1; filter_method = 0; interlace_method = 0; bit_on_pixel = 24; };
};
