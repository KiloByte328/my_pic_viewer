#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>
#include <math.h>

// может быть обратно перейти на строки
// можно вывести в глобальные переменные ИЛИ сделать одну функцию для проверки по нужной строке

/* bool sign_check_riff(const char* str, std::size_t size) {
    if (size < 5)
        return false;
    return str[0] == 'R' && str[1] == 'I' && str[2] == 'F' && str[3] == 'F';
}

bool scnd_chk_webp(const char* str, std::size_t size) {
    if (size < 6)
        return false;
    for (std::size_t i = 0; i <= size; i++) {
        if (str[i] == 'W' && str[i+1] == 'E' && str[i+2] == 'B' && (str[i+3] == str[i+5]) && str[i+3] == 'P' && str[i+4] == 'V' && str[i+6] == '8')
            return true;
        if (i == size)
            return false;
    }
    return false;
}

bool sign_check_jpeg(const char* str, std::size_t size) {
    if (size < 10)
        return false;
    return (str[0] == '\377' && str[2]  == '\377' && str[1] == '\330') && (str[6] == 'J' && str[7] == 'F' && str[9] == 'F' && str[8] == 'I');
}

bool sign_check_png(const char* str, std::size_t size) {
    if (size < 4)
        return false;
    return str[0] == '\211' && str[1] == 'P' && str[2] == 'N' && str[3] == 'G' && str[4] == '\r';
}

bool scnd_chk_png(const char* str, std::size_t size) {
    if (size < 2)
        return false;
    return str[0] == '\032' && str[1] == '\n';
} */

class Media_type{
    public:
    //virtual const char* get_meta_info() const = 0;
    virtual int get_type() const = 0;
    virtual int get_width() const = 0;
    virtual int get_height() const = 0;
    virtual std::size_t get_size() const = 0;
    virtual ~Media_type() = default;
};

class invalid_type : public Media_type {
    private:
    int type = 0;
    int width = 800;
    int height = 600;
    std::size_t size = 1;
    public:
    virtual int get_type() const override { return type; };
    virtual int get_width() const override { return width; };
    virtual int get_height() const override { return height; };
    virtual std::size_t get_size() const override { return size; };
};

class pic_png : public Media_type {
    private:
    int type = 1;
    int width = 800;
    int height = 600;
    std::size_t size = 1;
    public:
    virtual int get_type() const override { return type; };
    virtual int get_width() const override { return width; };
    virtual int get_height() const override { return height; };
    virtual std::size_t get_size() const override { return size; };
};

class pic_jpeg : public Media_type {
    private:
    int type = 2;
    int width = 800;
    int height = 600;
    std::size_t size = 1;
    public:
    virtual int get_type() const override { return type; };
    virtual int get_width() const override { return width; };
    virtual int get_height() const override { return height; };
    virtual std::size_t get_size() const override { return size; };
};

class pic_webp : public Media_type {
    private:
    int type = 3;
    int width = 800;
    int height = 600;
    std::size_t size = 1;
    public:
    virtual int get_type() const override { return type; };
    virtual int get_width() const override { return width; };
    virtual int get_height() const override { return height; };
    virtual std::size_t get_size() const override { return size; };
};

bool sign_check_str(const char* string_for_check, std::size_t size_of_string,
                    const char* string_for_find, std::size_t size_of_check_str) {
    if (size_of_string < size_of_check_str)
        return false;
    std::size_t counter = 0;
    for (std::size_t i = 0; i < size_of_string; i++) {
        if (string_for_check[i] == string_for_find[counter])
            counter++;
        if (counter == size_of_check_str)
            return true;
    }
    return false;
}

int main(int argc, char** argv) {
    std::string path, data;
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
    fl.open(path, std::ios::in | std::ios::binary);
    fl.read(data.data(), 200);
    std::cout << data.data() << '\n';
    data.find("\211PNG\r", 0) != data.npos ? std::cout << "PNG\n" : std::cout << "not PNG\n";
    sign_check_str(data.c_str(), 200, "\211PNG\r", 6) ? pntr = new pic_png : pntr = new invalid_type;
    pntr->get_type() == 1 ? std::cout << "PNG\n" : std::cout << "Not PNG\n";
    sign_check_str(data.c_str(), 200, "RIFF", 5) ? std::cout << "string check find RIFF\n" : std::cout << "string check dont find RIFF\n";
    sign_check_str(data.c_str(), 200, "\377\330\377", 4) ? std::cout << "string check find first half of jpeg sign\n" : std::cout << "string check dont find first half of jpeg sign\n";
    sign_check_str(data.c_str(), 200, "JFIF", 5) ? std::cout << "string check find second half of jpeg sign\n" : std::cout << "string check dont find second half of jpeg sign\n";
    fl.read(data.data(), 200);
    std::cout << data.c_str() << '\n';
    fl.read(data.data(), 200);
    std::cout << data.c_str() << '\n';
    fl.read(data.data(), 200);
    std::cout << data.c_str() << '\n';
    fl.close();
    delete pntr;
    return 0;
}