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

void fft_1d(complex_num* data, int n, int inverse) {
    // Bit-reverse copy
    int j = 0;
    for (int i = 0; i < n - 1; i++) {
        if (i < j) {
            complex_num temp = data[i];
            data[i] = data[j];
            data[j] = temp;
        }
        int k = n >> 1;
        while (k <= j) {
            j -= k;
            k >>= 1;
        }
        j += k;
    }

    // Compute FFT
    for (int step = 1; step < n; step <<= 1) {
        float angle = (inverse ? 2.0f : -2.0f) * M_PI / (2.0f * step);
        complex_num wn = {cosf(angle), sinf(angle)};
        
        for (int i = 0; i < n; i += 2 * step) {
            complex_num w = {1.0f, 0.0f};
            
            for (int j = 0; j < step; j++) {
                complex_num temp = prod(w, data[i + j + step]);
                data[i + j + step] = sum(data[i + j], opp(temp));
                data[i + j] = sum(data[i + j], temp);
                w = prod(w, wn);
            }
        }
    }

    // Scale if inverse
    if (inverse) {
        for (int i = 0; i < n; i++) {
            data[i].Rz /= n;
            data[i].Iz /= n;
        }
    }
}

void fft_2d(complex_num** data, int height, int width, int inverse) {
    // Allocate temporary arrays
    complex_num* row = (complex_num*)malloc(width * sizeof(complex_num));
    complex_num* col = (complex_num*)malloc(height * sizeof(complex_num));

    // Transform rows
    for (int i = 0; i < height; i++) {
        memcpy(row, data[i], width * sizeof(complex_num));
        fft_1d(row, width, inverse);
        memcpy(data[i], row, width * sizeof(complex_num));
    }

    // Transform columns
    for (int j = 0; j < width; j++) {
        for (int i = 0; i < height; i++) {
            col[i] = data[i][j];
        }
        fft_1d(col, height, inverse);
        for (int i = 0; i < height; i++) {
            data[i][j] = col[i];
        }
    }

    free(row);
    free(col);
}

// Function to compute frequency domain data (forward FFT)
FrequencyDomain* compute_frequency_domain(float** input, int width, int height) {
    FrequencyDomain* fd = create_frequency_domain(width, height);
    
    // Initialize with input data and zero padding
    for (int i = 0; i < fd->padded_height; i++) {
        for (int j = 0; j < fd->padded_width; j++) {
            if (i < height && j < width) {
                fd->data[i][j].Rz = input[i][j];
                fd->data[i][j].Iz = 0.0f;
            } else {
                fd->data[i][j].Rz = 0.0f;
                fd->data[i][j].Iz = 0.0f;
            }
        }
    }
    
    // Perform forward FFT
    fft_2d(fd->data, fd->padded_height, fd->padded_width, 0);
    
    return fd;
}

float** apply_frequency_filter(FrequencyDomain* fd, int N) {
    // Create temporary copy of frequency domain data
    complex_num** temp_data = (complex_num**)malloc(fd->padded_height * sizeof(complex_num*));
    for (int i = 0; i < fd->padded_height; i++) {
        temp_data[i] = (complex_num*)malloc(fd->padded_width * sizeof(complex_num));
        memcpy(temp_data[i], fd->data[i], fd->padded_width * sizeof(complex_num));
    }
    
    // Apply frequency domain filter
    for (int i = 0; i < fd->padded_height; i++) {
        for (int j = 0; j < fd->padded_width; j++) {
            int fi = i < fd->padded_height/2 ? i : i - fd->padded_height;
            int fj = j < fd->padded_width/2 ? j : j - fd->padded_width;
            
            if (abs(fi) > N || abs(fj) > N) {
                temp_data[i][j].Rz = 0.0f;
                temp_data[i][j].Iz = 0.0f;
            }
        }
    }
    
    // Perform inverse FFT
    fft_2d(temp_data, fd->padded_height, fd->padded_width, 1);
    
    // Copy result back to original size
    float** result = (float**)malloc(fd->original_height * sizeof(float*));
    for (int i = 0; i < fd->original_height; i++) {
        result[i] = (float*)malloc(fd->original_width * sizeof(float));
        for (int j = 0; j < fd->original_width; j++) {
            result[i][j] = temp_data[i][j].Rz;
        }
    }
    
    // Cleanup temporary data
    for (int i = 0; i < fd->padded_height; i++) {
        free(temp_data[i]);
    }
    free(temp_data);
    
    return result;
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
    FrequencyDomain* Y_freq = NULL;
    FrequencyDomain* Cb_freq = NULL;
    FrequencyDomain* Cr_freq = NULL;
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
    ///Compute Fds
    Y_freq = compute_frequency_domain(Y_array, width, height);
    Cb_freq = compute_frequency_domain(Cb, width, height);
    Cr_freq = compute_frequency_domain(Cr, width, height);
    unsigned char** magnitude_spectrum = (unsigned char**)malloc(Y_freq->padded_height * sizeof(unsigned char*));
    for (int i = 0; i < Y_freq->padded_height; i++) {
    magnitude_spectrum[i] = (unsigned char*)malloc(Y_freq->padded_width * sizeof(unsigned char));
    }
    compute_magnitude_spectrum(Y_freq, magnitude_spectrum);
    //Compute compression
    float** filteredY = apply_frequency_filter(Y_freq, compression_rate);
    float** filteredCb = apply_frequency_filter(Cb_freq, compression_rate/4);
    float** filteredCr = apply_frequency_filter(Cr_freq, compression_rate/4);
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
                        // Compression control
                        case SDLK_p:     
                            compression_rate++;
                            // Free previous filtered arrays
                            free_grayscale_array(filteredY, height);
                            free_grayscale_array(filteredCb, height);
                            free_grayscale_array(filteredCr, height);
                            
                            // Apply new filtering
                            filteredY = apply_frequency_filter(Y_freq, compression_rate);
                            filteredCb = apply_frequency_filter(Cb_freq, compression_rate/4);
                            filteredCr = apply_frequency_filter(Cr_freq, compression_rate/4);
                            
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
                }
            }
        }


        // Bottom right: frequency domain magnitude with position indicator
        for (int y = 0; y < hauteur/2; y++) {
            for (int x = 0; x < hauteur/2; x++) {
                // Map window coordinates to frequency domain coordinates
                int fd_y = y * Y_freq->padded_height / (hauteur/2);
                int fd_x = x * Y_freq->padded_width / (hauteur/2);
                
                if (fd_y < Y_freq->padded_height && fd_x < Y_freq->padded_width) {
                    unsigned char magnitude = magnitude_spectrum[fd_y][fd_x];
                    SDL_SetRenderDrawColor(rendu, magnitude, magnitude, magnitude, 255);
                    SDL_RenderDrawPoint(rendu, x+i, y + height+j);
                }
            }
        }

        SDL_RenderPresent(rendu);
    }

    free_rgb_array(rgb_array, height);
    free_grayscale_array(Y_array, height);
    free_grayscale_array(filteredY, height);
    free_grayscale_array(filteredCb, height);
    free_grayscale_array(filteredCr, height);
    free_frequency_domain(Y_freq);  
    free_frequency_domain(Cb_freq);
    free_frequency_domain(Cr_freq);
    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return EXIT_SUCCESS;
}
