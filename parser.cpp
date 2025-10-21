#include "parser_lib.hpp"

//garbage that i need, just magic_numbers
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
    fl.close();
    data.find("\211PNG\r", 0) != data.npos ? pntr = new pic_png(data) : pntr = new invalid_type;
    pntr->get_type() == 1 ? pntr->parse() : 0 ;
    std::cout << *pntr << '\n';
    delete pntr;
    return 0;
} 