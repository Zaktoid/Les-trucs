#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#pragma pack(push, 1)
typedef struct {
    uint16_t signature;    // "BM"
    uint32_t filesize;     // Size of the BMP file
    uint16_t reserved1;    // Reserved
    uint16_t reserved2;    // Reserved
    uint32_t dataOffset;   // Offset to image data
} BMPHeader;

typedef struct {
    uint32_t headerSize;      // Size of this header
    int32_t  width;          // Image width
    int32_t  height;         // Image height
    uint16_t planes;         // Number of color planes
    uint16_t bitsPerPixel;   // Bits per pixel
    uint32_t compression;    // Compression type
    uint32_t imageSize;      // Size of image data
    int32_t  xPixelsPerM;    // Horizontal resolution
    int32_t  yPixelsPerM;    // Vertical resolution
    uint32_t colorsUsed;     // Number of colors used
    uint32_t colorsImportant;// Important colors
} BMPInfoHeader;
#pragma pack(pop)

// Function to convert RGB to grayscale using luminosity method
double rgb_to_gray(uint8_t r, uint8_t g, uint8_t b) {
    return 0.299 * r + 0.587 * g + 0.114 * b;
}

// Function to compute total variation of an image
double compute_total_variation(const double* image, int width, int height, double h) {
    double total_variation = 0.0;
    
    // Iterate over each pixel
    for (int y = 0; y < height - 1; y++) {
        for (int x = 0; x < width - 1; x++) {
            // Compute horizontal and vertical differences
            double dx = (image[(y * width) + (x + 1)] - image[y * width + x]) / width;
            double dy = (image[((y + 1) * width) + x] - image[y * width + x]) / height;
            
            // Compute local variation using L2 norm
            double local_variation = sqrt(dx * dx + dy * dy);
            
            // Add to total variation
            total_variation += local_variation;
        }
    }
    
    return total_variation;
}

// Function to load BMP image and convert to grayscale
double* load_bmp_image(const char* filename, int* width, int* height) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Error opening file\n");
        return NULL;
    }
    
    // Read headers
    BMPHeader header;
    BMPInfoHeader infoHeader;
    
    fread(&header, sizeof(BMPHeader), 1, fp);
    fread(&infoHeader, sizeof(BMPInfoHeader), 1, fp);
    
    // Verify this is a BMP file
    if (header.signature != 0x4D42) { // "BM" in little endian
        fprintf(stderr, "Not a BMP file\n");
        fclose(fp);
        return NULL;
    }
    
    // Verify this is a 24-bit BMP
    if (infoHeader.bitsPerPixel != 24) {
        fprintf(stderr, "Only 24-bit BMP files are supported\n");
        fclose(fp);
        return NULL;
    }
    
    *width = infoHeader.width;
    *height = infoHeader.height;
    
    // Calculate row padding (BMP rows are padded to multiple of 4 bytes)
    int padding = (4 - ((*width * 3) % 4)) % 4;
    
    // Allocate memory for image data
    double* image = (double*)malloc(*width * *height * sizeof(double));
    if (!image) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(fp);
        return NULL;
    }
    
    // Seek to image data
    fseek(fp, header.dataOffset, SEEK_SET);
    
    // Read image data and convert to grayscale
    uint8_t pixel[3];
    for (int y = *height - 1; y >= 0; y--) {  // BMP is stored bottom-to-top
        for (int x = 0; x < *width; x++) {
            fread(pixel, 3, 1, fp);  // Read BGR
            // Convert to grayscale and normalize to [0,1]
            image[y * *width + x] = rgb_to_gray(pixel[2], pixel[1], pixel[0]) / 255.0;
        }
        // Skip padding bytes
        fseek(fp, padding, SEEK_CUR);
    }
    
    fclose(fp);
    return image;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <image.bmp>\n", argv[0]);
        return 1;
    }
    
    int width, height;
    const double h = 1;  // Step size for finite differences
    
    // Load image
    double* image = load_bmp_image(argv[1], &width, &height);
    if (!image) {
        return 1;
    }
    
    // Compute total variation
    double tv = compute_total_variation(image, width, height, h);
    printf("Total Variation: %f\n", tv);
    
    // Clean up
    free(image);
    return 0;
}