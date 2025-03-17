#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <stdint.h>
#include <string.h>

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} RGB;


// Function to load a JPEG image and store pixel values in an array
unsigned char* load_jpeg_image(const char* filename, int* width, int* height, int* channels) {
    // Initialize JPEG decompression object and error handler
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);

    // Open the JPEG file
    FILE* infile = fopen(filename, "rb");
    if (!infile) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return NULL;
    }

    // Initialize decompression
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    // Get image dimensions
    *width = cinfo.output_width;
    *height = cinfo.output_height;
    *channels = cinfo.output_components; // 1 for grayscale, 3 for RGB

    // Allocate memory for pixel data
    unsigned long data_size = (*width) * (*height) * (*channels);
    unsigned char* pixel_data = (unsigned char*)malloc(data_size);
    if (!pixel_data) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        return NULL;
    }

    // Read scanlines and store pixel data
    unsigned char* row_pointer = (unsigned char*)malloc(*width * *channels);
    if (!row_pointer) {
        fprintf(stderr, "Error: Memory allocation for row failed\n");
        free(pixel_data);
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        return NULL;
    }

    unsigned long location = 0;
    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, &row_pointer, 1);
        for (unsigned int i = 0; i < (*width) * (*channels); i++) {
            pixel_data[location++] = row_pointer[i];
        }
    }

    // Cleanup
    free(row_pointer);
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);

    return pixel_data;
}
////****************************************************************************************/ */
RGB** convert_to_2d_rgb(unsigned char* pixel_data, int width, int height, int channels) {
    // Allocate memory for rows
    RGB** rgb_array = (RGB**)malloc(height * sizeof(RGB*));
    if (!rgb_array) {
        fprintf(stderr, "Error: Memory allocation failed for rows\n");
        return NULL;
    }

    // Allocate memory for each row
    for (int i = 0; i < height; i++) {
        rgb_array[i] = (RGB*)malloc(width * sizeof(RGB));
        if (!rgb_array[i]) {
            // Clean up previously allocated memory
            for (int j = 0; j < i; j++) {
                free(rgb_array[j]);
            }
            free(rgb_array);
            fprintf(stderr, "Error: Memory allocation failed for columns\n");
            return NULL;
        }
    }

    // Fill the 2D array with RGB values
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int pixel_index = (y * width + x) * channels;
            
            if (channels == 3) {  // RGB image
                rgb_array[y][x].r = pixel_data[pixel_index];
                rgb_array[y][x].g = pixel_data[pixel_index + 1];
                rgb_array[y][x].b = pixel_data[pixel_index + 2];
            }
            else if (channels == 1) {  // Grayscale image
                rgb_array[y][x].r = pixel_data[pixel_index];
                rgb_array[y][x].g = pixel_data[pixel_index];
                rgb_array[y][x].b = pixel_data[pixel_index];
            }
        }
    }

    return rgb_array;
}

void free_rgb_array(RGB** rgb_array, int height) {
    if (rgb_array) {
        for (int i = 0; i < height; i++) {
            free(rgb_array[i]);
        }
        free(rgb_array);
    }
}
///**************************************************************************************// */
#pragma pack(push, 1)
// BMP file header structure
typedef struct {
    uint16_t signature;      // 'BM' for bitmap
    uint32_t fileSize;       // Size of the BMP file in bytes
    uint16_t reserved1;      // Reserved
    uint16_t reserved2;      // Reserved
    uint32_t dataOffset;     // Offset to image data in bytes
} BMPFileHeader;

// BMP info header structure
typedef struct {
    uint32_t headerSize;     // Size of this header in bytes (40)
    int32_t width;          // Width of the image in pixels
    int32_t height;         // Height of the image in pixels
    uint16_t planes;         // Number of color planes
    uint16_t bitsPerPixel;   // Bits per pixel (24 for RGB)
    uint32_t compression;    // Compression type (0 for uncompressed)
    uint32_t imageSize;      // Size of the image data in bytes
    int32_t xPixelsPerM;    // Horizontal resolution in pixels per meter
    int32_t yPixelsPerM;    // Vertical resolution in pixels per meter
    uint32_t colorsUsed;     // Number of colors used
    uint32_t colorsImportant;// Number of important colors
} BMPInfoHeader;
#pragma pack(pop)

// Function to save RGB array as BMP file
int save_as_bmp(const char* filename, RGB** rgb_array, int width, int height) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "Error: Cannot create file %s\n", filename);
        return 0;
    }

    // Calculate padding for rows
    int padding = (4 - (width * 3) % 4) % 4;
    int rowSize = width * 3 + padding;

    // Initialize file header
    BMPFileHeader fileHeader = {
        .signature = 0x4D42,  // 'BM' in little endian
        .fileSize = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + rowSize * height,
        .reserved1 = 0,
        .reserved2 = 0,
        .dataOffset = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader)
    };

    // Initialize info header
    BMPInfoHeader infoHeader = {
        .headerSize = sizeof(BMPInfoHeader),
        .width = width,
        .height = height,
        .planes = 1,
        .bitsPerPixel = 24,
        .compression = 0,
        .imageSize = rowSize * height,
        .xPixelsPerM = 3780,  // 96 DPI
        .yPixelsPerM = 3780,  // 96 DPI
        .colorsUsed = 0,
        .colorsImportant = 0
    };

    // Write headers
    fwrite(&fileHeader, sizeof(BMPFileHeader), 1, file);
    fwrite(&infoHeader, sizeof(BMPInfoHeader), 1, file);

    // Allocate memory for one row
    unsigned char* row = (unsigned char*)malloc(rowSize);
    if (!row) {
        fprintf(stderr, "Error: Memory allocation failed for row buffer\n");
        fclose(file);
        return 0;
    }

    // Write pixel data (bottom-up)
    for (int y = height - 1; y >= 0; y--) {
        int pos = 0;
        for (int x = 0; x < width; x++) {
            // BMP stores pixels in BGR order
            row[pos++] = rgb_array[y][x].b;
            row[pos++] = rgb_array[y][x].g;
            row[pos++] = rgb_array[y][x].r;
        }
        // Add padding
        for (int p = 0; p < padding; p++) {
            row[pos++] = 0;
        }
        fwrite(row, rowSize, 1, file);
    }

    // Clean up
    free(row);
    fclose(file);
    return 1;
}
/*********************************** */

int main(int argc, char *argv[]) {
    // Check if correct number of arguments provided
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_jpeg_file>\n", argv[0]);
        return 1;
    }

    const char* input_filename = argv[1];
    
    // Create output filename by replacing or appending .bmp extension
    char output_filename[256];
    strncpy(output_filename, input_filename, sizeof(output_filename) - 5); // -5 to ensure space for .bmp
    char *dot = strrchr(output_filename, '.');
    if (dot) {
        strcpy(dot, ".bmp");
    } else {
        strcat(output_filename, ".bmp");
    }

    int width, height, channels;
    
    // Load the JPEG image
    unsigned char* pixel_data = load_jpeg_image(input_filename, &width, &height, &channels);
    if (!pixel_data) {
        fprintf(stderr, "Error: Failed to load image %s\n", input_filename);
        return 1;
    }

    // Convert to 2D RGB array
    RGB** rgb_array = convert_to_2d_rgb(pixel_data, width, height, channels);
    if (!rgb_array) {
        fprintf(stderr, "Error: Failed to convert to 2D array\n");
        free(pixel_data);
        return 1;
    }

    // Save as BMP
    if (!save_as_bmp(output_filename, rgb_array, width, height)) {
        fprintf(stderr, "Error: Failed to save BMP file\n");
        free_rgb_array(rgb_array, height);
        free(pixel_data);
        return 1;
    }

    printf("Successfully converted %s to %s\n", input_filename, output_filename);

    // Clean up
    free_rgb_array(rgb_array, height);
    free(pixel_data);
    return 0;
}