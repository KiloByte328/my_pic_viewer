#pragma once
#include <string>

struct const_huff_val
{
    uint16_t value;
    uint16_t len;
};

class huffman_const {
    private:
        const_huff_val values[287];
};


void unpack(std::string data) {
    // если символ равен 256, то это конец для блока, если стоит BFINAL, то конец потока zlib deflate
    unsigned char symbol = 256;
    // так как мы идём задом наперёд, то будем считать с 7 до 0
    short bit = 0;
    bool is_final = 0;
    uint16_t code = 0;
    int value = -1;
    std::size_t data_cursor = 2;
    short huffman_code = 0;
    short range = 0; // 0 - 0-143, 1 - 144-255, 2 - 256-279, 3 - 280-287
    // если есть словарь
    if ((data[1] & 0x20) >> 5 == 1) {

    }
    while(data_cursor < data.size() - 4) {
        if ((data[data_cursor] & 0x01) == 1) 
            is_final = true;
        bit++;
        huffman_code = data[data_cursor] >> 1 & 0x03;
        bit += 2;
        while (true) {
            for (int bits_eated = 0; bits_eated <= 9; bit++, bits_eated) {
                code = (code << 1) | ((data[data_cursor] >> bit) & 1);
                if (bit == 7) {
                    data_cursor++;
                    bit = 0;
                }
                if (bits_eated == 7) {
                    if (code <= 23) {
                        value = code + 256;
                        range = 1;
                        break;
                    }
                }
                if (bits_eated == 8) {
                    if (code >= 48 && code <= 191) {
                        value = code - 48;
                        range = 0;
                        break;
                    }
                    if (code >= 192 && code <= 199) {
                        value = code - 192 + 280;
                        range = 3;
                        break;
                    }
                }
                if (bits_eated == 9) {
                    if (code >= 400 && code <= 511) {
                        value = code - 400 + 144;
                        range = 1;
                        break;
                    }
                }
            }
            if (value == -1) {
                break;
            }
        }
        // ^
        // переместить до while(true)
        // v
        switch (huffman_code)
        {
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
        if (is_final) 
            break;
    }

}