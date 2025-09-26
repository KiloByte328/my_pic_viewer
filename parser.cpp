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

int main(int argc, char** argv) {
    std::string path, data;
    std::stringstream str_strm;
    data.clear();
    path.clear();
    data.reserve(201);
    if (argc > 2) {
        std::cout << "Usage is: parser [path to file]\n";
        return 1;
    }
    if (argc == 2) {
        path.append(argv[1]);
    }
    if (argc < 2) {
        std::cout << "Enter path to media file\n";
        std::cin >> path;
    }
    std::fstream fl;
    Media_type* pntr;
    fl.open(path, std::ios::in);
    if (!fl.is_open()) {
        std::cout << "cant open file\n";
        return 1;
    }
    str_strm << fl.rdbuf();
    data.append(str_strm.str());
    data.find("\211PNG\r", 0) != data.npos ? pntr = (Media_type*) new pic_png : pntr = (Media_type*) new invalid_type;
    // find_in_c_str(data.c_str(), 200, "\211PNG\r", 6) != (std::size_t)-1
    //                ? pntr = (Media_type*) new pic_png : pntr = (Media_type*) new invalid_type;

    // pntr->get_type() == 1 ? std::cout << "PNG\n" : std::cout << "Not PNG\n";
    // find_in_c_str(data.c_str(), 200,
    //                "RIFF", 5) != (std::size_t)-1 ? std::cout << "string check find RIFF\n" :
    //                std::cout << "string check dont find RIFF\n";
    // find_in_c_str(data.c_str(), 200,
    //                "\377\330\377", 4) != (std::size_t)-1 ? std::cout << "string check find first half of jpeg sign\n" :
    //                std::cout << "string check dont find first half of jpeg sign\n";
    // find_in_c_str(data.c_str(), 200,
    //                "JFIF", 5) != (std::size_t)-1 ? std::cout << "string check find second half of jpeg sign\n" :
    //                std::cout << "string check dont find second half of jpeg sign\n";
    
    fl.close();
    delete pntr;
    return 0;
}