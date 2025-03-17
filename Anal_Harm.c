#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <time.h>
//#include <stdint.h>
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
#pragma pack(pop)
///*Consctructors and destructors*//////////

/*************** Global Variables ***************/
float s_global;

/*************** Function Declarations ***************/
void SDL_Exitwitherror(const char *message);
float sinc(float t, float s);
float phi_s(float t);
float Integ1D(float (*f)(float), float a, float b, int N);
float Integ2D(float (*f)(float, float), float a1, float b1, float a2, float b2, int N);
complex_num exp_2pi_inx(int n, float x);
float Convolute(float (*f)(float), float (*g)(float), float in, int N);
float LowPASS(float (*f)(float), float s, float t, int N);
float f(float x,float y);
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

/*************** Math Functions ***************/

/* Converts RGB to grayscale using luminance formula */
float rgb_to_gray(RGB pixel)
{
    return 0.299f * pixel.r + 0.587f * pixel.g + 0.114f * pixel.b;
}


void SDL_Exitwitherror(const char *message)
{
    SDL_Log("Erreur:%s >%s \n", message, SDL_GetError());
    exit(EXIT_FAILURE);
}

float sinc(float t, float s)
{
    if (t == 0) return 1.0;
    float x = (3.14) * t / s;
    return sin(x) / x;
}

float phi_s(float t)
{
    extern float s_global;
    return sinc(t, s_global);
}

float Integ1D(float (*f)(float), float a, float b, int N)
{
    if (N % 2 != 0) N++;
    float h = (b - a) / N;
    float integral = 0.0f;
    integral += f(a) + f(b);

    for (int i = 1; i < N; i++) {
        float x = a + i * h;
        integral += (i % 2 == 0) ? 2 * f(x) : 4 * f(x);
    }

    integral *= h / 3.0f;
    return integral;
}

float Integ2D(float (*f)(float, float), float a1, float b1, float a2, float b2, int N)
{
    float outer_integrand(float x)
    {
        float inner_integrand(float y)
        {
            return f(x, y);
        }
        return Integ1D(inner_integrand, a2, b2, N);
    }
    return Integ1D(outer_integrand, a1, b1, N);
}



float Convolute(float (*f)(float), float (*g)(float), float in, int N)
{
    float integrand(float t)
    {
        return f(t) * g(in - t);
    }
    float a = -10.0f;
    float b = 10.0f;
    return Integ1D(integrand, a, b, N);
}

float LowPASS(float (*f)(float), float s, float t, int N)
{
    extern float s_global;
    s_global = s;
    return Convolute(f, phi_s, t, N);
}

float f(float x, float y)
{
    if(x>0.25 && x<0.25 && y >0.25 && y<0.5 )
        return 1;
    return 0;
}


/*************** Main Function ***************/
int main(int argc, char **argv)
{
    srand(time(NULL));
    int hauteur = 800;
    int i = 0;  // Current viewing offset
    int j = 0;
    int s = 10;  // Pixels to move per keypress
    SDL_Window *window;
    SDL_Renderer *rendu;
    const char* input_filename = "output.bmp";
    int width, height;
    RGB** rgb_array = read_bmp(input_filename, &width, &height);
    if (!rgb_array) {
        fprintf(stderr, "Error: Failed to read BMP file\n");
        return 1;
    }
    if(SDL_Init(SDL_INIT_VIDEO))
        SDL_Exitwitherror("failed init");

    window = SDL_CreateWindow("Nom fenêtre", SDL_WINDOWPOS_CENTERED, 0, hauteur, hauteur, 0);
    if(!window)
        SDL_Exitwitherror("window creation failed");

    rendu = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(!rendu)
        SDL_Exitwitherror("renderer failed");

    SDL_bool Launched = SDL_TRUE;

    SDL_SetRenderDrawColor(rendu, 0, 0, 0, 255);

    ///////////////////// Main loop
    while (Launched) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    Launched = SDL_FALSE;
                    break;
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) 
                    {
                        case SDLK_UP:
                            j = j-s;
                            break;
                        case SDLK_DOWN:
                            j = j + s;
                            break;
                        case SDLK_LEFT:
                            i =i-s;
                            break;
                        case SDLK_RIGHT:
                            i = i + s;
                            break;
                        default:
                            break;
                    }
                default:
                    break;
            }
        }

        SDL_RenderClear(rendu);

        // Draw visible portion of image
        for (int y = 0; y < hauteur; y++) {
            for (int x = 0; x < hauteur; x++) {
                int imgY = y + j;
                int imgX = x + i;
                if (imgY < height && 0<imgY && 0<imgX && imgX < width) 
                {
                    SDL_SetRenderDrawColor(rendu, rgb_array[imgY][imgX].r, rgb_array[imgY][imgX].g, rgb_array[imgY][imgX].b, 255);
                }
                else  
                    SDL_SetRenderDrawColor(rendu, 0, 0,0, 255);
                        SDL_RenderDrawPoint(rendu, x, y);
                
            }
        }
       SDL_RenderPresent(rendu);
    }
//***********Cleanup***********//
    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}