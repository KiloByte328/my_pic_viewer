#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>

// const huffman lz77 map
const std::unordered_map<int, std::pair<int, int>> const_huffman_lz {
    {0, {0, 1}}, {1, {0, 2}}, {2, {0, 3}}, {3, {0, 4}}, {4, {1, 5}}, {5, {1, 7}},
    {6, {2, 9}}, {7, {2, 13}}, {8, {3, 17}}, {9, {3, 25}}, {10, {4, 33}}, {11, {4, 49}},
    {12, {5, 65}}, {13, {5, 97}}, {14, {6, 129}}, {15, {6, 193}}, {16, {7, 257}}, {17, {7, 385}},
    {18, {8, 513}}, {19, {8, 769}}, {20, {9, 1025}}, {21, {9, 1537}}, {22, {10, 2049}}, {23, {10, 3073}},
    {24, {11, 4097}}, {25, {11, 6415}}, {26, {12, 8193}}, {27, {12, 12289}}, {28, {13, 16385}}, {29, {13, 24577}}
};

struct zlib_data
{
    std::string data;
    std::size_t index;
    short last_read_bit;
};

uint32_t read_bits_from_bytes(zlib_data& data, uint32_t& bits_to_read) {
    uint32_t msg;
    for (uint32_t readed_bits = 0; readed_bits < bits_to_read;) {
        uint64_t left_to_read = bits_to_read - readed_bits;
        short new_data, left_in_byte = 8 - data.last_read_bit > left_to_read ? left_to_read : 8 - data.last_read_bit;
        msg = msg << left_in_byte;
        new_data = (data.data[data.index] << (8 - left_in_byte) ) >> (8 - left_in_byte);
        msg += new_data;
        data.last_read_bit += left_in_byte;
        if (data.last_read_bit >= 8) {
            data.index++;
            data.last_read_bit -= 8;
        }
        readed_bits += left_in_byte;
    }
    return msg;
}

void unpack_zlib(std::string& string_data) {
    zlib_data data {.data = string_data, .index = 0, .last_read_bit = 0};
    bool is_final = false;
    uint32_t code;
    std::vector<int> codes, distances;
    // если есть словарь
    if ((string_data[1] & 0x20) >> 5 == 1) {

    }
}

void unpack(std::string& data) {
    // если символ равен 256, то это конец для блока, если стоит BFINAL, то конец потока zlib deflate
    unsigned char symbol = 256;
    // так как мы идём задом наперёд, то будем считать с 7 до 0
    short bit = 0;
    // zlib_data new_data { .data = string_data, .index = 0, .last_read_bit = 0; };
    bool is_final = 0;
    uint16_t code = 0;
    std::vector<int> all_codes;
    std::vector<short> distances;
    uint16_t len_uncompressed;
    uint16_t nlen;
    std::pair<int, int> get_lz;
    short new_dist = 0;
    int value = -1;
    std::size_t data_cursor = 2;
    short huffman_code = 0;
    short range = 0; // 0 - 0-143, 1 - 144-255, 2 - 256-279, 3 - 280-287
    // если есть словарь
    if ((data[1] & 0x20) >> 5 == 1) {

    }
    // while (true) {
    //     if ((data[data_cursor] & 1) == 1)
    //         is_final = true;
    //     bit++;
    //     huffman_code = data[data_cursor] >> 1 & 0x03;
    //     bit += 2;
    //     
    // }
    while(data_cursor < data.size() - 4) {
        if ((data[data_cursor] & 0x01) == 1) 
            is_final = true;
        huffman_code = data[data_cursor] >> bit & 0x03;
        bit += 2;
        // ^
        // переместить до while(true)
        // v
        switch (huffman_code)
        {
        case 0:
        len_uncompressed = ((data[data_cursor+1]));
        len_uncompressed = len_uncompressed << 8;
        len_uncompressed += data[data_cursor+2];
        nlen = (data[data_cursor+3]);
        nlen << 8;
        nlen += data[data_cursor +4];
        if (len_uncompressed != !nlen) {
            std::cerr << "len not equal !nlen, error while transmitting\n";
            return;
        }
        data_cursor += 4;
        // no huffman codes, just copy
        // read len - two bytes and read negative len next two bytes, if len != ~nlen then data corrupted 
        break;
        case 1:
        // fixed huffman, huffman_const
            while (true) {
                for (int bits_eated = 0; bits_eated <= 8; bit++, bits_eated++) {
                    code = (code << 1) | ((data[data_cursor] >> bit) & 1);
                    if (bit == 8) {
                        data_cursor++;
                        bit = 0;
                    }
                    if (bits_eated == 6) {
                        if (code <= 23) {
                            value = code + 256;
                            range = 1;
                            break;
                        }
                    }
                    if (bits_eated == 7) {
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
                    if (bits_eated == 8) {
                        if (code >= 400 && code <= 511) {
                            value = code - 400 + 144;
                            range = 1;
                            for (short r = 0; r < 5; r++, bit++) {
                                if (bit == 8) {
                                    data_cursor++;
                                    bit = 0;
                                }
                                new_dist = (new_dist << 1) | ((data[data_cursor] >> bit) & 1);
                            }
                            if (const_huffman_lz.find(new_dist) == const_huffman_lz.end()) {
                                std::cerr << "find not existed lz77 match at " << data_cursor <<  "\n";
                                return;
                                //error, maybe throw exception maybe not
                            }
                            get_lz = (*const_huffman_lz.find(new_dist)).second;
                            new_dist = 0;
                            for (short q = 0; q < get_lz.first; q++, bit++) {
                                if (bit == 8) {
                                    data_cursor++;
                                    bit = 0;
                                }
                                new_dist = (new_dist << 1) | ((data[data_cursor] >> bit) & 1);
                            }
                            new_dist += get_lz.second;
                            distances.push_back(new_dist);
                            break;
                        }
                    }
                }
                if (value == -1 || data_cursor == data.size() - 4) {
                    break;
                }
                all_codes.push_back(value);
                code = 0;
                new_dist = 0;
            }
        break;
        case 2:
        // dynamic huffman, need to read 5 bits, 5 bits, 4 bits 
        break;
        default:
        //error, reserved
        break;
        }
        if (is_final) 
            break;
    }
    std::cout << "codes\n";
    for (auto& a : all_codes) {
        std::cout << a << '\n';
    }
    std::cout << "distances\n";
    for (auto& d : distances) {
        std::cout << d << '\n';
    }

}