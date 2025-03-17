#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <time.h>
#include <stdint.h>
#include <fftw3.h>
#include "Mathutils.h"

/*************** Structures ***************/
#pragma pack(push, 1)
typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} RGB;

typedef struct {
    uint32_t headerSize;      // Size of this header in bytes (40)
    int32_t width;           // Width of the image in pixels
    int32_t height;          // Height of the image in pixels
    uint16_t planes;         // Number of color planes
    uint16_t bitsPerPixel;   // Bits per pixel (24 for RGB)
    uint32_t compression;    // Compression type (0 for uncompressed)
    uint32_t imageSize;      // Size of the image data in bytes
    int32_t xPixelsPerM;    // Horizontal resolution in pixels per meter
    int32_t yPixelsPerM;    // Vertical resolution in pixels per meter
    uint32_t colorsUsed;     // Number of colors used
    uint32_t colorsImportant;// Number of important colors
} BMPInfoHeader;

typedef struct {
    uint16_t signature;      // 'BM' for bitmap
    uint32_t fileSize;       // Size of the BMP file in bytes
    uint16_t reserved1;      // Reserved
    uint16_t reserved2;      // Reserved
    uint32_t dataOffset;     // Offset to image data in bytes
} BMPFileHeader;
typedef struct {
    float* real;
    float* imag;
    int width;
    int height;
} DFT_Result;
typedef struct {
    complex_num** data;
    int padded_width;
    int padded_height;
    int original_width;
    int original_height;
} FrequencyDomain;
#pragma pack(pop)
/////////////////Helpers
int next_power_of_2(int n) 
{
    int power = 1;
    while (power < n) 
    {
        power *= 2;
    }
    return power;
}
void compute_magnitude_spectrum(FrequencyDomain* fd, unsigned char** spectrum) {
    int center_i = fd->padded_height / 2;
    int center_j = fd->padded_width / 2;
    
    // Find maximum magnitude for normalization
    float max_magnitude = 0.0f;
    for (int i = 0; i < fd->padded_height; i++) {
        for (int j = 0; j < fd->padded_width; j++) {
            complex_num c = fd->data[i][j];
            float magnitude = sqrtf(c.Rz * c.Rz + c.Iz * c.Iz);
            if (magnitude > max_magnitude) max_magnitude = magnitude;
        }
    }
    
    // Compute normalized spectrum with FFT shift
    for (int i = 0; i < fd->padded_height; i++) {
        for (int j = 0; j < fd->padded_width; j++) {
            // Shift indices to center DC component
            int i_shifted = (i + center_i) % fd->padded_height;
            int j_shifted = (j + center_j) % fd->padded_width;
            
            complex_num c = fd->data[i][j];
            float magnitude = sqrtf(c.Rz * c.Rz + c.Iz * c.Iz);
            // Log scale and normalize
            float normalized = log1pf(magnitude) / log1pf(max_magnitude);
            spectrum[i_shifted][j_shifted] = (unsigned char)(normalized * 255.0f);
        }
    }
}
///*Consctructors and destructors*//////////
FrequencyDomain* create_frequency_domain(int width, int height) {
    FrequencyDomain* fd = (FrequencyDomain*)malloc(sizeof(FrequencyDomain));
    fd->original_width = width;
    fd->original_height = height;
    fd->padded_width = next_power_of_2(width);
    fd->padded_height = next_power_of_2(height);
    
    fd->data = (complex_num**)malloc(fd->padded_height * sizeof(complex_num*));
    for (int i = 0; i < fd->padded_height; i++) {
        fd->data[i] = (complex_num*)malloc(fd->padded_width * sizeof(complex_num));
    }
    
    return fd;
}
void free_frequency_domain(FrequencyDomain* fd) {
    if (fd) {
        for (int i = 0; i < fd->padded_height; i++) {
            free(fd->data[i]);
        }
        free(fd->data);
        free(fd);
    }
}

/*************** Global Variables ***************/
float s_global;

/*************** Function Declarations ***************/
void SDL_Exitwitherror(const char *message);
void free_rgb_array(RGB** rgb_array, int height);
RGB** read_bmp(const char* filename, int* width, int* height);

/*************** Memory Management Functions ***************/
void free_rgb_array(RGB** rgb_array, int height)
{
    if (rgb_array) {
        for (int i = 0; i < height; i++) {
            free(rgb_array[i]);
        }
        free(rgb_array);
    }
}
void free_grayscale_array(float** array, int height) {
    if (array) {
        for (int i = 0; i < height; i++) {
            free(array[i]);
        }
        free(array);
    }
}
void free_chrom_array(float** array, int height) {
    if (array) {
        for (int i = 0; i < height; i++) {
            free(array[i]);
        }
        free(array);
    }
}
//////////////////
typedef struct {
    float* psi;  // Wavelet function (mother wavelet)
    float* phi;  // Scaling function (father wavelet)
    int length;  // Length of the functions
} HaarBasis;

typedef struct {
    float** coefficients;
    int levels;
    int width;
    int height;
    HaarBasis* bases;  // Array of bases for each level
} WaveletTransform;

// Helper functions for Haar wavelet transform
void haar_decompose_1d(float* data, int length) {
    float* temp = (float*)malloc(length * sizeof(float));
    int half = length >> 1;
    
    // Compute approximation and detail coefficients
    for (int i = 0; i < half; i++) {
        int idx = i << 1;
        temp[i] = (data[idx] + data[idx + 1]) * 0.7071067811865476f;  // 1/sqrt(2)
        temp[half + i] = (data[idx] - data[idx + 1]) * 0.7071067811865476f;
    }
    
    // Copy results back to input array
    memcpy(data, temp, length * sizeof(float));
    free(temp);
}

void haar_reconstruct_1d(float* data, int length) {
    float* temp = (float*)malloc(length * sizeof(float));
    int half = length >> 1;
    
    // Reconstruct signal from approximation and detail coefficients
    for (int i = 0; i < half; i++) {
        int idx = i << 1;
        temp[idx] = (data[i] + data[half + i]) * 0.7071067811865476f;
        temp[idx + 1] = (data[i] - data[half + i]) * 0.7071067811865476f;
    }
    
    memcpy(data, temp, length * sizeof(float));
    free(temp);
}

WaveletTransform* create_wavelet_transform(int width, int height, int levels) {
    WaveletTransform* wt = (WaveletTransform*)malloc(sizeof(WaveletTransform));
    wt->width = width;
    wt->height = height;
    wt->levels = levels;
    
    // Allocate coefficients array
    wt->coefficients = (float**)malloc(height * sizeof(float*));
    for (int i = 0; i < height; i++) {
        wt->coefficients[i] = (float*)malloc(width * sizeof(float));
    }
    
    return wt;
}

void free_wavelet_transform(WaveletTransform* wt) {
    if (wt) {
        for (int i = 0; i < wt->height; i++) {
            free(wt->coefficients[i]);
        }
        free(wt->coefficients);
        free(wt);
    }
}

// 2D Haar wavelet transform
void haar_transform_2d(WaveletTransform* wt, float** input) {
    int current_width = wt->width;
    int current_height = wt->height;
    float* row = (float*)malloc(wt->width * sizeof(float));
    float* col = (float*)malloc(wt->height * sizeof(float));
    
    // Copy input to coefficients
    for (int i = 0; i < wt->height; i++) {
        memcpy(wt->coefficients[i], input[i], wt->width * sizeof(float));
    }
    
    // Perform decomposition for each level
    for (int level = 0; level < wt->levels && current_width > 1 && current_height > 1; level++) {
        // Transform rows
        for (int i = 0; i < current_height; i++) {
            memcpy(row, wt->coefficients[i], current_width * sizeof(float));
            haar_decompose_1d(row, current_width);
            memcpy(wt->coefficients[i], row, current_width * sizeof(float));
        }
        
        // Transform columns
        for (int j = 0; j < current_width; j++) {
            for (int i = 0; i < current_height; i++) {
                col[i] = wt->coefficients[i][j];
            }
            haar_decompose_1d(col, current_height);
            for (int i = 0; i < current_height; i++) {
                wt->coefficients[i][j] = col[i];
            }
        }
        
        current_width >>= 1;
        current_height >>= 1;
    }
    
    free(row);
    free(col);
}

void haar_inverse_transform_2d(WaveletTransform* wt) {
    float* row = (float*)malloc(wt->width * sizeof(float));
    float* col = (float*)malloc(wt->height * sizeof(float));
    
    int current_width = wt->width >> (wt->levels - 1);
    int current_height = wt->height >> (wt->levels - 1);
    
    // Perform reconstruction for each level
    for (int level = wt->levels - 1; level >= 0; level--) {
        // Transform columns
        for (int j = 0; j < current_width; j++) {
            for (int i = 0; i < current_height; i++) {
                col[i] = wt->coefficients[i][j];
            }
            haar_reconstruct_1d(col, current_height);
            for (int i = 0; i < current_height; i++) {
                wt->coefficients[i][j] = col[i];
            }
        }
        
        // Transform rows
        for (int i = 0; i < current_height; i++) {
            memcpy(row, wt->coefficients[i], current_width * sizeof(float));
            haar_reconstruct_1d(row, current_width);
            memcpy(wt->coefficients[i], row, current_width * sizeof(float));
        }
        
        current_width <<= 1;
        current_height <<= 1;
    }
    
    free(row);
    free(col);
}

// Threshold function for compression
void apply_wavelet_threshold(WaveletTransform* wt, float threshold) {
    for (int i = 0; i < wt->height; i++) {
        for (int j = 0; j < wt->width; j++) {
            if (fabs(wt->coefficients[i][j]) < threshold) {
                wt->coefficients[i][j] = 0.0f;
            }
        }
    }
}

// Main compression function
float** apply_wavelet_compression(float** input, int width, int height, int levels, float threshold) {
    WaveletTransform* wt = create_wavelet_transform(width, height, levels);
    
    // Forward transform
    haar_transform_2d(wt, input);
    
    // Apply threshold for compression
    apply_wavelet_threshold(wt, threshold);
    
    // Inverse transform
    haar_inverse_transform_2d(wt);
    
    // Copy result to output array
    float** result = (float**)malloc(height * sizeof(float*));
    for (int i = 0; i < height; i++) {
        result[i] = (float*)malloc(width * sizeof(float));
        memcpy(result[i], wt->coefficients[i], width * sizeof(float));
    }
    
    free_wavelet_transform(wt);
    return result;
}
/////////////////////////////////


/*************** BMP File Handling ***************/

RGB** read_bmp(const char* filename, int* width, int* height)
{
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return NULL;
    }

    BMPFileHeader fileHeader;
    if (fread(&fileHeader, sizeof(BMPFileHeader), 1, file) != 1) {
        fprintf(stderr, "Error: Failed to read BMP file header\n");
        fclose(file);
        return NULL;
    }

    if (fileHeader.signature != 0x4D42) {
        fprintf(stderr, "Error: Not a valid BMP file\n");
        fclose(file);
        return NULL;
    }

    BMPInfoHeader infoHeader;
    if (fread(&infoHeader, sizeof(BMPInfoHeader), 1, file) != 1) {
        fprintf(stderr, "Error: Failed to read BMP info header\n");
        fclose(file);
        return NULL;
    }

    if (infoHeader.bitsPerPixel != 24 || infoHeader.compression != 0) {
        fprintf(stderr, "Error: Unsupported BMP format. Only 24-bit uncompressed supported\n");
        fclose(file);
        return NULL;
    }

    *width = infoHeader.width;
    *height = infoHeader.height;

    int padding = (4 - ((*width) * 3) % 4) % 4;
    int rowSize = (*width) * 3 + padding;

    RGB** rgb_array = (RGB**)malloc((*height) * sizeof(RGB*));
    if (!rgb_array) {
        fprintf(stderr, "Error: Memory allocation failed for rows\n");
        fclose(file);
        return NULL;
    }

    for (int i = 0; i < *height; i++) {
        rgb_array[i] = (RGB*)malloc((*width) * sizeof(RGB));
        if (!rgb_array[i]) {
            for (int j = 0; j < i; j++) {
                free(rgb_array[j]);
            }
            free(rgb_array);
            fclose(file);
            fprintf(stderr, "Error: Memory allocation failed for columns\n");
            return NULL;
        }
    }

    fseek(file, fileHeader.dataOffset, SEEK_SET);

    unsigned char* row = (unsigned char*)malloc(rowSize);
    if (!row) {
        free_rgb_array(rgb_array, *height);
        fclose(file);
        fprintf(stderr, "Error: Memory allocation failed for row buffer\n");
        return NULL;
    }

    for (int y = *height - 1; y >= 0; y--) {
        if (fread(row, rowSize, 1, file) != 1) {
            free(row);
            free_rgb_array(rgb_array, *height);
            fclose(file);
            fprintf(stderr, "Error: Failed to read pixel data\n");
            return NULL;
        }

        for (int x = 0; x < *width; x++) {
            int pos = x * 3;
            rgb_array[y][x].b = row[pos];
            rgb_array[y][x].g = row[pos + 1];
            rgb_array[y][x].r = row[pos + 2];
        }
    }

    free(row);
    fclose(file);
    return rgb_array;
}

/*************** Image Processing Functions ***************/


void SDL_Exitwitherror(const char *message)
{
    SDL_Log("Erreur:%s >%s \n", message, SDL_GetError());
    exit(EXIT_FAILURE);
}



float RgbToY(RGB pixel) {
    return 16.0f + (0.299f * pixel.r + 0.587f * pixel.g + 0.114f * pixel.b);
}
float** RgbArrayToY(RGB** pixels, int width, int height) {
    float** y_array = (float**)malloc(height * sizeof(float*));
    if (!y_array) return NULL;
    
    for (int i = 0; i < height; i++) {
        y_array[i] = (float*)malloc(width * sizeof(float));
        if (!y_array[i]) {
            for (int j = 0; j < i; j++) {
                free(y_array[j]);
            }
            free(y_array);
            return NULL;
        }
        
        for (int j = 0; j < width; j++) {
            y_array[i][j] = RgbToY(pixels[i][j]);
        }
    }
    
    return y_array;
}

float RgbToCb(RGB pixel) {
    return 128.0f + (-0.169f * pixel.r - 0.331f * pixel.g + 0.5f * pixel.b);
}

float RgbToCr(RGB pixel) {
    return 128.0f + (0.5f * pixel.r - 0.419f * pixel.g - 0.081f * pixel.b);
}


float** RgbArrayToChrom(RGB** pixels, int width, int height, short is_Cb) {
    float** chrom_array = (float**)malloc(height * sizeof(float*));
    if (!chrom_array) return NULL;
    
    for (int i = 0; i < height; i++) {
        chrom_array[i] = (float*)malloc(width * sizeof(float));
        if (!chrom_array[i]) {
            for (int j = 0; j < i; j++) {
                free(chrom_array[j]);
            }
            free(chrom_array);
            return NULL;
        }
        
        for (int j = 0; j < width; j++) {
            chrom_array[i][j] = is_Cb ? RgbToCb(pixels[i][j]) : RgbToCr(pixels[i][j]);
        }
    }
    
    return chrom_array;
}



RGB ChromToRGB(float Y, float Cb, float Cr) {
    RGB pixel;
    
    // Denormalize Y
    float y_scaled = (Y - 16.0f) * 1.164f;
    
    // Convert to RGB using BT.601 coefficients
    float r = y_scaled + 1.596f * (Cr - 128.0f);
    float g = y_scaled - 0.813f * (Cr - 128.0f) - 0.391f * (Cb - 128.0f);
    float b = y_scaled + 2.018f * (Cb - 128.0f);
    
    // Clamp values and round properly
    pixel.r = (unsigned char)fmax(0.0f, fmin(255.0f, roundf(r)));
    pixel.g = (unsigned char)fmax(0.0f, fmin(255.0f, roundf(g)));
    pixel.b = (unsigned char)fmax(0.0f, fmin(255.0f, roundf(b)));
    
    return pixel;
}

RGB** ChromArrayToRGB(float** Y_array, float** Cb_array, float** Cr_array, int width, int height) {
    RGB** rgb_array = (RGB**)malloc(height * sizeof(RGB*));
    if (!rgb_array) return NULL;
    for (int i = 0; i < height; i++) {
        rgb_array[i] = (RGB*)malloc(width * sizeof(RGB));
        if (!rgb_array[i]) {
            for (int j = 0; j < i; j++) {
                free(rgb_array[j]);
            }
            free(rgb_array);
            return NULL;
        }
        for (int j = 0; j < width; j++) {
            rgb_array[i][j] = ChromToRGB(Y_array[i][j], Cb_array[i][j], Cr_array[i][j]);
        }
    }
    
    return rgb_array;
}







////////////////////////////////////////////////////////////////////////
void normalize_coefficients_for_display(float** coefficients, unsigned char** display, int width, int height) {
    // Find min and max values
    float min_val = coefficients[0][0];
    float max_val = coefficients[0][0];
    
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (coefficients[i][j] < min_val) min_val = coefficients[i][j];
            if (coefficients[i][j] > max_val) max_val = coefficients[i][j];
        }
    }
    
    // Normalize to [0, 255]
    float range = max_val - min_val;
    if (range == 0) range = 1;  // Avoid division by zero
    
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            display[i][j] = (unsigned char)(255.0f * (coefficients[i][j] - min_val) / range);
        }
    }
}

// Function to allocate 2D array for coefficient display
unsigned char** allocate_display_buffer(int width, int height) {
    unsigned char** buffer = (unsigned char**)malloc(height * sizeof(unsigned char*));
    for (int i = 0; i < height; i++) {
        buffer[i] = (unsigned char*)malloc(width * sizeof(unsigned char));
    }
    return buffer;
}

void free_display_buffer(unsigned char** buffer, int height) {
    if (buffer) {
        for (int i = 0; i < height; i++) {
            free(buffer[i]);
        }
        free(buffer);
    }

}


/*************** Main Function ***************/
int main(int argc, char **argv) {
    srand(time(NULL));
    int hauteur = 1000;
    int i = 0;
    int j = 0;
    int s = 10;
    int a = 0;  // Frequency parameter for x
    int b = 0;  // Frequency parameter for y
    SDL_Window *window;
    SDL_Renderer *rendu;
    const char* input_filename;
    int compression_rate;
    
    // Check if filename was provided as argument
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <bmp_filename>\n", argv[0]);
        return 1;
    }
    
    input_filename = argv[1];
    compression_rate = atoi(argv[2]);
    int width, height;
    
    // Read the BMP file
    RGB** rgb_array = read_bmp(input_filename, &width, &height);
    if (!rgb_array) 
    {
        fprintf(stderr, "Error: Failed to read BMP file %s\n", input_filename);
        return 1;
    }

    // Convert to grayscale
    float** Y_array = RgbArrayToY(rgb_array, width, height);
    float** Cb = RgbArrayToChrom(rgb_array, width, height, 1);
    float** Cr = RgbArrayToChrom(rgb_array, width, height, 0);

    //Compute compression
    float** filteredY = apply_wavelet_compression(Y_array,width,height,3,compression_rate/100.0);
    float** filteredCb = apply_wavelet_compression(Cb,width,height,3,compression_rate/100.0);
    float** filteredCr = apply_wavelet_compression(Cr,width,height,3,compression_rate/100.0);
    unsigned char** coeff_display = allocate_display_buffer(width, height);
    WaveletTransform* wt = create_wavelet_transform(width, height, 3);
    haar_transform_2d(wt, Y_array); 
    normalize_coefficients_for_display(wt->coefficients, coeff_display, width, height);
    RGB** CompreSSed = ChromArrayToRGB(filteredY, filteredCb, filteredCr, width, height);
	if(SDL_Init(SDL_INIT_VIDEO))
		SDL_Exitwitherror("failed init");
	window= SDL_CreateWindow("Nom fenêtre",SDL_WINDOWPOS_CENTERED,0,hauteur,hauteur,0);			
	if(!window)
		SDL_Exitwitherror("window creation failed");
	rendu = SDL_CreateRenderer(window,-1,SDL_RENDERER_SOFTWARE);
	if(!rendu)
		SDL_Exitwitherror("renderer failed");
	SDL_bool Launched= SDL_TRUE;
    
    // Main loop
    while (Launched) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    Launched = SDL_FALSE;
                    break;
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_UP:    j = j-s; break;
                        case SDLK_DOWN:  j = j+s; break;
                        case SDLK_LEFT:  i = i-s; break;
                        case SDLK_RIGHT: i = i+s; break;
                        case SDLK_z:     b--; break;  // Decrement y frequency (Z key)
                        case SDLK_s:     b++; break;  // Increment y frequency (S key)
                        case SDLK_q:     a--; break;  // Decrement x frequency (Q key)
                        case SDLK_d:     a++; break;  // Increment x frequency (D key)
                        case SDLK_p:     
                            compression_rate++;
                            // Free previous filtered arrays
                            free_grayscale_array(filteredY, height);
                            free_grayscale_array(filteredCb, height);
                            free_grayscale_array(filteredCr, height);
                            
                            // Apply new filtering
                            filteredY = apply_wavelet_compression(Y_array,width,height,3,compression_rate);
                            filteredCb = apply_wavelet_compression(Cb,width,height,3,compression_rate);
                            filteredCr = apply_wavelet_compression(Cr,width,height,3,compression_rate);
                            
                            // Free previous compressed image
                            free_rgb_array(CompreSSed, height);
                            
                            // Create new compressed image
                            CompreSSed = ChromArrayToRGB(filteredY, filteredCb, filteredCr, width, height);
                            break;
                        
                        default: break;
                    }
                default: break;
            }       
        }

        // Clear screen
        SDL_SetRenderDrawColor(rendu, 0, 0, 0, 255);
        SDL_RenderClear(rendu);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int imgY = y;
                int imgX = x;
                
                if (imgY < height && imgY >= 0 && imgX >= 0 && imgX < width) {
                    // Compressed image (right)
                    SDL_SetRenderDrawColor(rendu, 
                        CompreSSed[imgY][imgX].r,
                        CompreSSed[imgY][imgX].g,
                        CompreSSed[imgY][imgX].b,
                        255);
                    SDL_RenderDrawPoint(rendu, x + i+width, y+j);
                    
                    // Original image (left)
                    SDL_SetRenderDrawColor(rendu,
                        rgb_array[imgY][imgX].r,
                        rgb_array[imgY][imgX].g,
                        rgb_array[imgY][imgX].b,
                        255);
                    SDL_RenderDrawPoint(rendu, x+i, y+j);
                    unsigned char coeff_val = coeff_display[imgY][imgX];
                    SDL_SetRenderDrawColor(rendu, coeff_val, coeff_val, coeff_val, 255);
                    SDL_RenderDrawPoint(rendu, x + i, y + j + height);
                }
            }
        }



        SDL_RenderPresent(rendu);
    }

    free_grayscale_array(Y_array, height);
    free_grayscale_array(filteredY, height);
    free_grayscale_array(filteredCb, height);
    free_grayscale_array(filteredCr, height);
    free_rgb_array(CompreSSed,height);
    free_rgb_array(rgb_array,height);
    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return EXIT_SUCCESS;
}
