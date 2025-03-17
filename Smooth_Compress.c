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

float** filterFrequencies(float** grayscale, int width, int height, int N) {
    fftwf_complex *in = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * width * height);
    fftwf_complex *out = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * width * height);
    
    // Create plans
    fftwf_plan forward = fftwf_plan_dft_2d(height, width, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    fftwf_plan backward = fftwf_plan_dft_2d(height, width, out, in, FFTW_BACKWARD, FFTW_ESTIMATE);
    
    // Copy input and apply window function
    for(int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {
            in[i * width + j][0] = grayscale[i][j];
            in[i * width + j][1] = 0.0;
        }
    }
    
    // Forward FFT
    fftwf_execute(forward);
    
    // Filter with smooth transition
    float center_i = height / 2.0f;
    float center_j = width / 2.0f;
    float N_float = (float)N;
    
    for(int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {
            float di = i < height/2 ? i : i - height;
            float dj = j < width/2 ? j : j - width;
            float radius = sqrtf(di * di + dj * dj);
            
            // Smooth transition
            float filter = 1.0f;
            if(radius > N_float) {
                filter = expf(-(radius - N_float) * 0.1f);
            }
            
            out[i * width + j][0] *= filter;
            out[i * width + j][1] *= filter;
        }
    }
    
    // Inverse FFT
    fftwf_execute(backward);
    
    // Copy back with proper normalization
    float** filtered = (float**)malloc(height * sizeof(float*));
    for(int i = 0; i < height; i++) {
        filtered[i] = (float*)malloc(width * sizeof(float));
        for(int j = 0; j < width; j++) {
            filtered[i][j] = in[i * width + j][0] / (width * height);
        }
    }
    
    // Cleanup
    fftwf_destroy_plan(forward);
    fftwf_destroy_plan(backward);
    fftwf_free(in);
    fftwf_free(out);
    
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
    const char* input_filename = "Peach_dress.bmp";
    int width, height;
    
    // Read the BMP file
    RGB** rgb_array = read_bmp(input_filename, &width, &height);
    if (!rgb_array) {
        fprintf(stderr, "Error: Failed to read BMP file\n");
        return 1;
    }

    // Convert to grayscale
    float** Y_array = RgbArrayToY(rgb_array, width, height);
    float** Cb = RgbArrayToChrom(rgb_array, width, height, 1);
    float** Cr = RgbArrayToChrom(rgb_array, width, height, 0);
    //Compute compression
    float** filteredY = filterFrequencies(Y_array, width, height, 100);  // Keep more Y detail
    float** filteredCb = filterFrequencies(Cb, width, height, 50);       // Reduce Cb detail
    float** filteredCr = filterFrequencies(Cr, width, height, 50);
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
