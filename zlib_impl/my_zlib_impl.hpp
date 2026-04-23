#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>

const std::unordered_map<int, std::pair<int, int>> const_huffman_distance {
    {0, {0, 1}}, {1, {0, 2}}, {2, {0, 3}}, {3, {0, 4}}, {4, {1, 5}}, {5, {1, 7}},
    {6, {2, 9}}, {7, {2, 13}}, {8, {3, 17}}, {9, {3, 25}}, {10, {4, 33}}, {11, {4, 49}},
    {12, {5, 65}}, {13, {5, 97}}, {14, {6, 129}}, {15, {6, 193}}, {16, {7, 257}}, {17, {7, 385}},
    {18, {8, 513}}, {19, {8, 769}}, {20, {9, 1025}}, {21, {9, 1537}}, {22, {10, 2049}}, {23, {10, 3073}},
    {24, {11, 4097}}, {25, {11, 6415}}, {26, {12, 8193}}, {27, {12, 12289}}, {28, {13, 16385}}, {29, {13, 24577}}
};

const std::unordered_map<int, std::pair<int, int>> const_huffman_length {
    {257, {0, 3}}, {258, {0, 4}}, {259, {0, 5}}, {260, {0, 6}}, {261, {0, 7}}, {262, {0, 8}},
    {263, {0, 9}}, {264, {0, 10}}, {265, {1, 11}}, {266, {1, 13}}, {267, {1, 15}}, {268, {1, 17}},
    {269, {2, 19}}, {270, {2, 23}}, {271, {2, 27}}, {272, {2, 31}}, {273, {3, 35}}, {274, {3, 43}},
    {275, {3, 51}}, {276, {3, 59}}, {277, {4, 67}}, {278, {4, 83}}, {279, {4, 99}}, {280, {4, 115}},
    {281, {5, 131}}, {282, {5, 163}}, {283, {5, 195}}, {284, {5, 227}}, {285, {0, 258}}
};

struct zlib_data
{
    std::string data;
    std::size_t index;
    short last_read_bit;
};

uint32_t read_bits_msb(zlib_data& data, uint32_t bits_to_read) {
    uint32_t msg = 0;
    for (uint32_t readed_bits = 0; readed_bits < bits_to_read; readed_bits++) {
        if (data.last_read_bit == 8) {
            data.index++;
            data.last_read_bit = 0;
        }
        msg = (msg << 1) | (((unsigned char) data.data[data.index] >> data.last_read_bit) & 1);
        data.last_read_bit++;
    }
    return msg;
}

uint32_t read_bits_lsb(zlib_data& data, uint32_t bits_to_read) {
    uint32_t msg = 0;
    for (int64_t readed_bits = bits_to_read; readed_bits > 0; readed_bits--) {
        if (data.last_read_bit == 8) {
            data.index++;
            data.last_read_bit = 0;
        }
        short bit = ((((unsigned char) data.data[data.index] >> data.last_read_bit) & 1));
        msg = msg | (bit << (readed_bits - 1));
        // std::cout << "get bit " << readed_bits-1 << " from lsb: " << bit << "\nmessage is: " << msg << '\n';
        data.last_read_bit++;
    }
    return msg;
}

void unpack_zlib(std::string& string_data) {
    if (string_data.size() < 2) {
        std::cout << "data can not be empty or below 2 chars\n";
        return;
    }
    zlib_data data {.data = string_data, .index = 1, .last_read_bit = 3};
    bool is_final = false;
    uint32_t code = 0;
    uint16_t distance = 0;
    uint64_t length = 0;
    short check = 0;
    std::vector<MyMediaTypes::MediaTypePixels> rgba;
    std::vector<uint8_t> unpacked_bytes;
    std::vector<int> codes, lengths, distances;
    // если есть словарь
    if (read_bits_lsb(data, 1) == 1) {
        // adler32 checksum 
    }
    data.index = 2;
    data.last_read_bit = 0;
    while (true) {
        if (((data.data[data.index] >> data.last_read_bit) & 1) == 1) {
            is_final = true;
        }
        data.last_read_bit++;
        if (data.last_read_bit == 8) {
            data.index++;
            data.last_read_bit = 0;
        }
        check = (data.data[data.index] >> data.last_read_bit) & 3;
        data.last_read_bit += 2;
        switch (check) {
            case 0:
            break;
            case 1:
                while(true) {
                    code = 0;
                    length = 0;
                    distance = 0;
                    code = read_bits_lsb(data, 7);
                    if (code <= 23) {
                        code += 256;
                        if (code == 256) {
                            break;
                        }
                        if (const_huffman_length.find(code) != const_huffman_length.end()) {
                            std::pair<int, int> get_length = (*const_huffman_length.find(code)).second;
                            length = read_bits_lsb(data, get_length.first) + get_length.second;
                            lengths.push_back(length);
                            distance = read_bits_msb(data, 5);
                            get_length = (*const_huffman_distance.find(distance)).second;
                            distance = read_bits_lsb(data, get_length.first) + get_length.second;
                            distances.push_back(distance);
                        }
                        codes.push_back(code);
                        if (const_huffman_length.find(code) != const_huffman_length.end()) {
                            codes.push_back(length);
                            codes.push_back(distance);
                        }
                        continue;
                    }
                    code = (code << 1) | read_bits_lsb(data, 1);
                    if (code >= 48 && code <= 191) {
                        code = code - 48;
                        codes.push_back(code);
                        continue;
                    }
                    else if (code >= 192 && code <= 199) {
                        code = code - 192 + 280;
                        if (const_huffman_length.find(code) != const_huffman_length.end()) {
                            std::pair<int, int> get_length = (*const_huffman_length.find(code)).second;
                            length = read_bits_lsb(data, get_length.first) + get_length.second;
                            lengths.push_back(length);
                            distance = read_bits_msb(data, 5);
                            get_length = (*const_huffman_distance.find(distance)).second;
                            distance = read_bits_lsb(data, get_length.first) + get_length.second;
                            distances.push_back(distance);
                        }
                        codes.push_back(code);
                        if (const_huffman_length.find(code) != const_huffman_length.end()) {
                            codes.push_back(length);
                            codes.push_back(distance);
                        }
                        continue;
                    }
                    code = (code << 1) | read_bits_lsb(data, 1);
                    if (code >= 400 && code <= 511) {
                        code = code - 400 + 144;
                        codes.push_back(code);
                    }
                    else {
                        break;
                    }
                }
            case 2:
            break;
        }
        if (is_final) break;
    }
    for (auto& c : codes) {
        std::cout << "code: " << c << '\n';
    }
    for (std::size_t q = 0; q < lengths.size(); q++) {
        std::cout << "length: " << lengths[q] << " distance: " << distances[q] << '\n';
    }
    for (std::size_t i = 0; i < codes.size(); i++) {
        if (codes[i] >= 256) {
            std::cout << "i above 255 is " << i << '\n' << "codes i+1 is " << codes[i+1] << " codes i+2 is " << codes[i+2]
                      << " size of inpacked codes is " << unpacked_bytes.size() << '\n';
            for (int q = unpacked_bytes.size() - codes[i+2], readed = 0; readed < codes[i+1]; readed++) {
                unpacked_bytes.push_back(unpacked_bytes[q + (readed % (uint64_t)codes[i+2])]);
                std::cout << "readed is " << readed << " q is " << q << " readed % codes[i+2] " << readed % codes[i+2] << '\n';
            }
            i += 2;
        }
        else {
            unpacked_bytes.push_back(codes[i]);
        }
    }
    for (auto& b : unpacked_bytes) {
        std::cout << (int)b << '\n';
    }
}

// void unpack(std::string& data) {
//     // если символ равен 256, то это конец для блока, если стоит BFINAL, то конец потока zlib deflate
//     unsigned char symbol = 256;
//     // так как мы идём задом наперёд, то будем считать с 7 до 0
//     short bit = 0;
//     // zlib_data new_data { .data = string_data, .index = 0, .last_read_bit = 0; };
//     bool is_final = 0;
//     uint16_t code = 0;
//     std::vector<int> all_codes;
//     std::vector<short> distances;
//     uint16_t len_uncompressed;
//     uint16_t nlen;
//     std::pair<int, int> get_lz;
//     short new_dist = 0;
//     int value = -1;
//     std::size_t data_cursor = 2;
//     short huffman_code = 0;
//     short range = 0; // 0 - 0-143, 1 - 144-255, 2 - 256-279, 3 - 280-287
//     // если есть словарь
//     if ((data[1] & 0x20) >> 5 == 1) {

//     }
//     // while (true) {
//     //     if ((data[data_cursor] & 1) == 1)
//     //         is_final = true;
//     //     bit++;
//     //     huffman_code = data[data_cursor] >> 1 & 0x03;
//     //     bit += 2;
//     //     
//     // }
//     while(data_cursor < data.size() - 4) {
//         if ((data[data_cursor] & 0x01) == 1) 
//             is_final = true;
//         huffman_code = data[data_cursor] >> bit & 0x03;
//         bit += 2;
//         switch (huffman_code)
//         {
//         case 0:
//         len_uncompressed = ((data[data_cursor+1]));
//         len_uncompressed = len_uncompressed << 8;
//         len_uncompressed += data[data_cursor+2];
//         nlen = (data[data_cursor+3]);
//         nlen << 8;
//         nlen += data[data_cursor +4];
//         if (len_uncompressed != !nlen) {
//             std::cerr << "len not equal !nlen, error while transmitting\n";
//         }
//         data_cursor += 4;
//         // no huffman codes, just copy
//         // read len - two bytes and read negative len next two bytes, if len != ~nlen then data corrupted while transmitting
//         break;
//         case 1:
//         // fixed huffman, huffman_const
//             while (true) {
//                 for (int bits_eated = 0; bits_eated <= 8; bit++, bits_eated++) {
//                     if (bit == 8) {
//                         data_cursor++;
//                         bit = 0;
//                     }
//                     code = (code << 1) | ((data[data_cursor] >> bit) & 1);
//                     if (bits_eated == 6) {
//                         if (code <= 23) {
//                             value = code + 256;
//                             range = 1;
//                             break;
//                         }
//                     }
//                     if (bits_eated == 7) {
//                         if (code >= 48 && code <= 191) {
//                             value = code - 48;
//                             range = 0;
//                             break;
//                         }
//                         if (code >= 192 && code <= 199) {
//                             value = code - 88;
//                             range = 3;
//                             break;
//                         }
//                     }
//                     if (bits_eated == 8) {
//                         if (code >= 400 && code <= 511) {
//                             bit++;
//                             new_dist = 0;
//                             value = code - 256;
//                             range = 1;
//                             for (short r = 0; r < 5; r++, ++bit) {
//                                 if (bit == 8) {
//                                     data_cursor++;
//                                     bit = 0;
//                                 }
//                                 new_dist = (new_dist << 1) | ((data[data_cursor] >> bit) & 1);
//                             }
//                             if (const_huffman_length.find(new_dist) == const_huffman_length.end()) {
//                                 std::cerr << "find not existed lz77 match at " << data_cursor 
//                                 <<  "\n" << "match was: " << new_dist << "\ncode was: " << code << '\n';
//                                 return;
//                                 //error, maybe throw exception maybe not
//                             }
//                             get_lz = (*const_huffman_length.find(new_dist)).second;
//                             new_dist = 0;
//                             for (short q = 0; q < get_lz.first; q++, bit++) {
//                                 if (bit == 8) {
//                                     data_cursor++;
//                                     bit = 0;
//                                 }
//                                 new_dist = (new_dist << 1) | ((data[data_cursor] >> bit) & 1);
//                             }
//                             new_dist += get_lz.second;
//                             distances.push_back(new_dist);
//                             break;
//                         }
//                     }
//                 }
//                 if (value == -1 || data_cursor == data.size() - 4) {
//                     break;
//                 }
//                 all_codes.push_back(value);
//                 code = 0;
//                 new_dist = 0;
//             }
//         break;
//         case 2:
//         // dynamic huffman, need to read 5 bits, 5 bits, 4 bits 
//         break;
//         default:
//         //error, reserved
//         break;
//         }
//         if (is_final) 
//             break;
//     }
//     std::cout << "codes\n";
//     for (auto& a : all_codes) {
//         std::cout << a << '\n';
//     }
//     std::cout << "distances\n";
//     for (auto& d : distances) {
//         std::cout << d << '\n';
//     }

// }