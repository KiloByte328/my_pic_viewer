#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>
#include <math.h>
#include <sstream>

class Media_type{
    private:
    int type, width, height;
    std::size_t size;
    std::string data;
    bool corrupted = false;
    public:
    virtual int get_type() const { return type; };
    virtual int get_width() const { return width; };
    virtual int get_height() const { return height; };
    virtual std::size_t get_size() const { return size; };
    virtual std::string get_data() const {return data; };
    virtual void add_data(std::string new_data) { data.append(new_data); };
    virtual ~Media_type() = default;
    Media_type() { data.clear(); }
};

class invalid_type : private Media_type {
    private:
    int type = 0;
    int width = 800;
    int height = 600;
    std::size_t size = 1;
    public:
};

// PNG

// PNG chucnks struct: IDHR -> 
// cHRM/gAMA/iCCP xor sRGB/mDCV/cLLI/dBIT/eXLf/pHYs/sPLT/acTL -> 
// bKGD/hIST/tRNS/eXLf/pHYs/sPLT/acTL -> PLTE(optional) ->
// IDAT -> tIME/iTXt/tEXt/zTXt -> IEND

class pic_png : private Media_type {
    private:
    char multiple_chunks_allowed[7][5] = {"IDAT", "fcTL", "sPLT", "fdAT", "iTXt", "tEXt", "zTXt"};
    int type = 1;
    int width = 800;
    int height = 600;
    std::size_t size = 1, end_of_pic;
    public:
};

// JPEG
class pic_jpeg : private Media_type {
    private:
    int type = 2;
    int width = 800;
    int height = 600;
    std::size_t size = 1;
    public:
};

// RIFF *File size* WEBP
class pic_webp : private Media_type {
    private:
    int type = 3;
    int width = 800;
    int height = 600;
    std::size_t size = 1;
    public:
};
