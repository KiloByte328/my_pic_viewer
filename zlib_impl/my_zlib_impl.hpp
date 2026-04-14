#pragma once
#include <string>

class huffman {
    private:

};



void unpack(std::string data) {
    // если символ равен 256, то это конец для блока, если стоит BFINAL, то конец потока zlib deflate
    unsigned char symbol = 256;
    // так как мы идём задом наперёд, то будем считать с 7 до 0
    short bit = 7;
    bool is_final = 0;
    uint16_t code = 0;
    std::size_t data_cursor = 2;
    // если есть словарь
    if ((data[1] & 0x20) >> 5 == 1) {

    }
    while(data_cursor < data.size() - 4) {
        if (data[data_cursor] & 0x01 == 1) 
            is_final = true;
        bit--;
        switch ((data[data_cursor] >> 1) & 0x03) {
        case 0:
        // no huffman codes, just copy
        break;
        case 1:
        // fixed huffman, table
        break;
        case 2:
        // dynamic huffman, need to read 5 bits, 5 bits, 4 bits 
        break;
        default:
        //error, reserved
        }
    }

}