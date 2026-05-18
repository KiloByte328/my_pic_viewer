import zlib
import struct

def make_chunk(chunk_type, data):
    chunk = struct.pack(">I", len(data)) + chunk_type + data
    crc = zlib.crc32(chunk_type + data) & 0xffffffff
    chunk += struct.pack(">I", crc)
    return chunk

# Настройки картинки (16x16, RGB, 8-bit)
width, height = 4, 4
bit_depth = 8
color_type = 2 # RGB

# 1. Заголовок IHDR
ihdr_data = struct.pack(">IIBBBBB", width, height, bit_depth, color_type, 0, 0, 0)
ihdr_chunk = make_chunk(b"IHDR", ihdr_data)

# 2. Сырые данные пикселей
# Красный цвет: R=255, G=0, B=0. 
# Каждая строка начинается с байта фильтра (0 = None)
row_data = b'\x00' + (b'\x00\x00\xff' * width)
raw_image_data = row_data * height

# СЖАТИЕ С ПРИНУДИТЕЛЬНЫМ FIXED HUFFMAN (BTYPE = 1)
# Уровень сжатия 9, DEFLATED, окно 32k, память 8, СТРАТЕГИЯ Z_FIXED
compressor = zlib.compressobj(9, zlib.DEFLATED, 15, 8, zlib.Z_FIXED)
compressed_data = compressor.compress(raw_image_data) + compressor.flush()

# 3. Чанк IDAT
idat_chunk = make_chunk(b"IDAT", compressed_data)

# 4. Конец IEND
iend_chunk = make_chunk(b"IEND", b"")

# Записываем в файл
with open("fixed_huffman_test.png", "wb") as f:
    f.write(b"\x89PNG\r\n\x1a\n") # PNG Signature
    f.write(ihdr_chunk)
    f.write(idat_chunk)
    f.write(iend_chunk)

print("Файл fixed_huffman_test.png успешно создан!")