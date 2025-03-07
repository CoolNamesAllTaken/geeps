#pragma once

#include "sd_utils.hh"

static const uint16_t kBitsPerByte = 8;
static const uint16_t kBytesPerWord = 4;
static const uint16_t kBMPFileHeaderSizeBytes = 14;

inline bool ReadBMPDimensions(const char *filename, uint16_t &width, uint16_t &height) {
    FIL bmp_file;
    FRESULT fr = f_open(&bmp_file, filename, FA_READ);
    if (fr != FR_OK) {
        printf("ReadBMPDimensions: Failed to open file %s: %s (%d)\n", filename, FRESULT_str(fr), fr);
        return false;
    }

    // Read the BMP header.
    uint8_t bmp_header[54];
    UINT bytes_read;
    fr = f_read(&bmp_file, bmp_header, sizeof(bmp_header), &bytes_read);
    if (fr != FR_OK) {
        printf("ReadBMPDimensions: Failed to read BMP header: %s (%d)\n", FRESULT_str(fr), fr);
        f_close(&bmp_file);
        return false;
    }

    // Check if the file is a BMP.
    if (bmp_header[0] != 'B' || bmp_header[1] != 'M') {
        printf("ReadBMPDimensions: File is not a BMP.\n");
        f_close(&bmp_file);
        return false;
    }

    // Get the image dimensions.
    width = *(uint16_t *)&bmp_header[0x12];
    height = *(uint16_t *)&bmp_header[0x16];

    f_close(&bmp_file);
    return true;
}

inline bool ReadBMPToBuffer(const char *filename, uint8_t *buffer, uint16_t buffer_size, uint16_t width,
                            uint16_t height) {
    FIL bmp_file;
    FRESULT fr = f_open(&bmp_file, filename, FA_READ);
    if (fr != FR_OK) {
        printf("ReadBMPToBuffer: Failed to open file %s: %s (%d)\n", filename, FRESULT_str(fr), fr);
        return false;
    }

    // Read the BMP header.
    uint8_t bmp_header[kBMPFileHeaderSizeBytes];
    UINT bytes_read;
    fr = f_read(&bmp_file, bmp_header, sizeof(bmp_header), &bytes_read);
    if (fr != FR_OK) {
        printf("ReadBMPToBuffer: Failed to read BMP header: %s (%d)\n", FRESULT_str(fr), fr);
        f_close(&bmp_file);
        return false;
    }

    // Check if the file is a BMP.
    if (bmp_header[0] != 'B' || bmp_header[1] != 'M') {
        printf("ReadBMPToBuffer: File is not a BMP.\n");
        f_close(&bmp_file);
        return false;
    }

    // Get the image size.
    uint32_t image_size_pixels = (width + (kBitsPerByte - width % kBitsPerByte) % kBitsPerByte) * height / kBitsPerByte;
    // *(uint32_t *)&bmp_header[0x02] / (3 * kBitsPerByte);  // Convert 24-bit depth to 1-bit depth.
    if (image_size_pixels > buffer_size) {
        printf("ReadBMPToBuffer: Image size %lu is larger than buffer size %u.\n", image_size_pixels, buffer_size);
        f_close(&bmp_file);
        return false;
    }

    // Map each pixel in the BMP to bits in the buffer. Any non-zero value is considered black.
    uint32_t image_data_offset = *(uint32_t *)&bmp_header[0x0A];
    // Purge unused data into buffer.
    f_read(&bmp_file, buffer, image_data_offset - kBMPFileHeaderSizeBytes, &bytes_read);
    // Clear the image buffer.
    memset(buffer, 0xff, buffer_size);
    // Read pixel array row by row.
    // Rows in the BMP are padded to the nearest word.
    uint16_t buffer_bytes_per_row = width / kBitsPerByte + (width % kBitsPerByte != 0);
    for (uint16_t row = 0; row < height; row++) {
        uint16_t col = 0;
        for (; col < width; col++) {
            uint8_t pixel_data[3];
            f_read(&bmp_file, pixel_data, sizeof(pixel_data), &bytes_read);
            if (bytes_read != sizeof(pixel_data)) {
                printf("ReadBMPToBuffer: Failed to read pixel data: %s (%d)\n", FRESULT_str(fr), fr);
                f_close(&bmp_file);
                return false;
            }
            uint8_t pixel_value = (pixel_data[0] + pixel_data[1] + pixel_data[2]) > 0 ? 0 : 1;
            // Reverse rows since image is stored upside down.
            uint16_t buffer_index = (height - 1 - row) * buffer_bytes_per_row + col / kBitsPerByte;
            uint8_t bit_index = col % kBitsPerByte;
            buffer[buffer_index] &= ~(pixel_value << (kBitsPerByte - 1 - bit_index));
            printf("%c", pixel_value ? '#' : '0');  // Print image as it is read (this will be upside down).
        }
        while (col % kBytesPerWord != 0) {
            uint8_t padding_byte;
            f_read(&bmp_file, &padding_byte, sizeof(padding_byte), &bytes_read);
            if (bytes_read != sizeof(padding_byte)) {
                printf("ReadBMPToBuffer: Failed to read padding byte: %s (%d)\n", FRESULT_str(fr), fr);
                f_close(&bmp_file);
                return false;
            }
            col++;
        }
        printf("\n");
    }

    if (f_close(&bmp_file) != FR_OK) {
        printf("ReadBMPToBuffer: Failed to close file: %s (%d)\n", FRESULT_str(fr), fr);
        return false;
    }
    return true;
}