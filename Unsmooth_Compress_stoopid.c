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
#pragma pack(pop)
///*Consctructors and destructors*//////////
DFT_Result* create_dft_result(int width, int height) {
    DFT_Result* result = (DFT_Result*)malloc(sizeof(DFT_Result));
    if (!result) return NULL;
    
    result->width = width;
    result->height = height;
    result->real = (float*)calloc(width * height, sizeof(float));
    result->imag = (float*)calloc(width * height, sizeof(float));
    
    if (!result->real || !result->imag) {
        free(result->real);
        free(result->imag);
        free(result);
        return NULL;
    }
    
    return result;
}
void free_dft_result(DFT_Result* result) {
    if (result) {
        free(result->real);
        free(result->imag);
        free(result);
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
void naive_dft_1d(complex_num* data_time, complex_num* data_freq, int n) {
    for (int k = 0; k < n; k++) {
        data_freq[k].Rz = 0;
        data_freq[k].Iz = 0;
        
        for (int m = 0; m < n; m++) {
            float angle = -2.0f * M_PI * k * m / n;
            complex_num exp_factor = {cosf(angle), sinf(angle)};
            complex_num temp = prod(data_time[m], exp_factor);
            data_freq[k].Rz += temp.Rz;
            data_freq[k].Iz += temp.Iz;
        }
    }
}

// Naive implementation of 2D forward DFT
void naive_dft_2d(complex_num** data_time, complex_num** data_freq, int height, int width) {
    // Temporary storage for intermediate results
    complex_num** temp = (complex_num**)malloc(height * sizeof(complex_num*));
    for (int i = 0; i < height; i++) {
        temp[i] = (complex_num*)malloc(width * sizeof(complex_num));
    }
    
    // First pass: transform rows
    for (int i = 0; i < height; i++) {
        naive_dft_1d(data_time[i], temp[i], width);
    }
    
    // Allocate column arrays
    complex_num* col_time = (complex_num*)malloc(height * sizeof(complex_num));
    complex_num* col_freq = (complex_num*)malloc(height * sizeof(complex_num));
    
    // Second pass: transform columns
    for (int j = 0; j < width; j++) {
        // Extract column
        for (int i = 0; i < height; i++) {
            col_time[i] = temp[i][j];
        }
        
        // Transform column
        naive_dft_1d(col_time, col_freq, height);
        
        // Put back transformed column
        for (int i = 0; i < height; i++) {
            data_freq[i][j] = col_freq[i];
        }
    }
    
    // Cleanup
    free(col_time);
    free(col_freq);
    for (int i = 0; i < height; i++) {
        free(temp[i]);
    }
    free(temp);
}

// Naive implementation of 1D inverse DFT
void naive_idft_1d(complex_num* data_freq, complex_num* data_time, int n) {
    for (int k = 0; k < n; k++) {
        data_time[k].Rz = 0;
        data_time[k].Iz = 0;
        
        for (int m = 0; m < n; m++) {
            float angle = 2.0f * M_PI * k * m / n;
            complex_num exp_factor = {cosf(angle), sinf(angle)};
            complex_num temp = prod(data_freq[m], exp_factor);
            data_time[k].Rz += temp.Rz / n;
            data_time[k].Iz += temp.Iz / n;
        }
    }
}

// Naive implementation of 2D inverse DFT
void naive_idft_2d(complex_num** data_freq, complex_num** data_time, int height, int width) {
    // Temporary storage for intermediate results
    complex_num** temp = (complex_num**)malloc(height * sizeof(complex_num*));
    for (int i = 0; i < height; i++) {
        temp[i] = (complex_num*)malloc(width * sizeof(complex_num));
    }
    
    // First pass: transform rows
    for (int i = 0; i < height; i++) {
        naive_idft_1d(data_freq[i], temp[i], width);
    }
    
    // Allocate column arrays
    complex_num* col_freq = (complex_num*)malloc(height * sizeof(complex_num));
    complex_num* col_time = (complex_num*)malloc(height * sizeof(complex_num));
    
    // Second pass: transform columns
    for (int j = 0; j < width; j++) {
        // Extract column
        for (int i = 0; i < height; i++) {
            col_freq[i] = temp[i][j];
        }
        
        // Transform column
        naive_idft_1d(col_freq, col_time, height);
        
        // Put back transformed column
        for (int i = 0; i < height; i++) {
            data_time[i][j] = col_time[i];
        }
    }
    
    // Cleanup
    free(col_freq);
    free(col_time);
    for (int i = 0; i < height; i++) {
        free(temp[i]);
    }
    free(temp);
}

// Modified filterFrequencies function using naive IDFT
float** filterFrequencies_naive(float** grayscale, int width, int height, int N) {
    // Allocate complex arrays
    complex_num** data_freq = (complex_num**)malloc(height * sizeof(complex_num*));
    complex_num** data_time = (complex_num**)malloc(height * sizeof(complex_num*));
    for(int i = 0; i < height; i++) {
        data_freq[i] = (complex_num*)malloc(width * sizeof(complex_num));
        data_time[i] = (complex_num*)malloc(width * sizeof(complex_num));
        for(int j = 0; j < width; j++) {
            data_freq[i][j].Rz = grayscale[i][j];
            data_freq[i][j].Iz = 0.0f;
        }
    }
    
    // Forward transform using naive DFT
    naive_dft_2d(data_freq, data_time, height, width);
    
    // Copy result back to data_freq for filtering
    for(int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {
            data_freq[i][j] = data_time[i][j];
        }
    }
    
    // Filter frequencies
    for(int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {
            int fi = i < height/2 ? i : i - height;
            int fj = j < width/2 ? j : j - width;
            
            if(abs(fi) > N || abs(fj) > N) {
                data_freq[i][j].Rz = 0.0f;
                data_freq[i][j].Iz = 0.0f;
            }
        }
    }
    
    // Inverse transform using naive IDFT
    naive_idft_2d(data_freq, data_time, height, width);
    
    // Copy real parts to output
    float** filtered = (float**)malloc(height * sizeof(float*));
    for(int i = 0; i < height; i++) {
        filtered[i] = (float*)malloc(width * sizeof(float));
        for(int j = 0; j < width; j++) {
            filtered[i][j] = data_time[i][j].Rz;
        }
    }
    
    // Cleanup
    for(int i = 0; i < height; i++) {
        free(data_freq[i]);
        free(data_time[i]);
    }
    free(data_freq);
    free(data_time);
    
    return filtered;
}


/*************** Main Function ***************/
int main(int argc, char **argv) {
    srand(time(NULL));
    int hauteur = 1000;
    int i = 0;
    int j = 0;
    int s = 10;
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
    float** filteredY = filterFrequencies_naive(Y_array, width, height, compression_rate);  // Keep more Y detail
    float** filteredCb = filterFrequencies_naive(Cb, width, height, compression_rate/4);       // Reduce Cb detail
    float** filteredCr = filterFrequencies_naive(Cr, width, height,compression_rate/4);
    RGB** CompreSSed=ChromArrayToRGB(filteredY,filteredCb,filteredCr,width,height);
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
                        default: break;
                    }
                default: break;
            }
        }

        // Clear screen
        SDL_SetRenderDrawColor(rendu, 0, 0, 0, 255);
        SDL_RenderClear(rendu);

        for (int y = 0; y < hauteur; y++) {
            for (int x = 0; x < hauteur; x++) {
                int imgY = y;
                int imgX = x;
                
                if (imgY < height && imgY >= 0 && imgX >= 0 && imgX < width) {
                    SDL_SetRenderDrawColor
                    (
                        rendu,
                        CompreSSed[imgY][imgX].r,
                        CompreSSed[imgY][imgX].g,
                        CompreSSed[imgY][imgX].b,
                        255
                    );
                    SDL_RenderDrawPoint(rendu, x+i+width, y+j);
                    SDL_SetRenderDrawColor
                    (
                        rendu,
                        rgb_array[imgY][imgX].r,
                        rgb_array[imgY][imgX].g,
                        rgb_array[imgY][imgX].b,
                        255
                    );
                    SDL_RenderDrawPoint(rendu, x+i, y+j);
                    
                }
            }
        }
        



        SDL_RenderPresent(rendu);
    }

    // Cleanup
    free_rgb_array(rgb_array, height);
    free_grayscale_array(Y_array, height);
    free_grayscale_array(filteredY, height);
    free_grayscale_array(filteredCb, height);
    free_grayscale_array(filteredCr, height);
    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return EXIT_SUCCESS;
}
