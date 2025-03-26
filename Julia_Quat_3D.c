#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <omp.h>
#include <math.h>
#include "Mathutils.h"

quat C={-0.4, 0.6, 0, 0};

// Structure to store volumetric data
typedef struct {
    float *data;
    int width;
    int height;
    int depth;
} VolumeData;

void SDL_Exitwitherror(const char *message)
{
    SDL_Log("Erreur:%s >%s \n", message, SDL_GetError());
    exit(EXIT_FAILURE);
}

float TestDiverg(int tres, quat (*f)(quat), quat a, double toler)
{
    quat interation = a;
    for(int k = 0; k < tres; k++)
    {
        interation = f(interation);
        if(module_q(interation) > toler)
            return k;
    }
    return 0;
}

quat Mendel(quat in)
{
    return sum_q(Pow_q(in, 2), C);
}

// Function to generate 3D volumetric data of the Julia set
VolumeData* generateJulia3D(int size, float horiz, float vert, float depth_offset, 
                           float z, int tres, quat (*fractalFunc)(quat))
{
    VolumeData *volume = (VolumeData*)malloc(sizeof(VolumeData));
    if (!volume) {
        fprintf(stderr, "Failed to allocate memory for volume structure\n");
        return NULL;
    }
    
    volume->width = size;
    volume->height = size;
    volume->depth = size;
    
    // Allocate memory for volumetric data
    volume->data = (float*)malloc(size * size * size * sizeof(float));
    if (!volume->data) {
        fprintf(stderr, "Failed to allocate memory for volumetric data\n");
        free(volume);
        return NULL;
    }
    
    // Calculate Julia set in parallel for 3D volume
    #pragma omp parallel for collapse(3)
    for(int x = 0; x < size; x++) {
        for(int y = 0; y < size; y++) {
            for(int z_idx = 0; z_idx < size; z_idx++) {
                // Map voxel coordinates to quaternion space (using x, y, z components)
                // The fourth quaternion component is fixed at 0 to create a 3D slice
                quat a = {
                    ((double)x - (size/2) + horiz) / (z * (size/4)),
                    ((double)y - (size/2) + vert) / (z * (size/4)),
                    ((double)z_idx - (size/2) + depth_offset) / (z * (size/4)),
                    0  // Fourth component fixed at 0 (this is where we "slice" the 4D object)
                };
                
                float divergence = TestDiverg(5 * z * tres, fractalFunc, a, 5 * z * tres);
                
                // Store result in 3D buffer
                int idx = x + y * size + z_idx * size * size;
                volume->data[idx] = divergence;
            }
        }
    }
    
    return volume;
}

// Function to render a 2D slice from the 3D volume
void renderVolumeSlice(SDL_Renderer *renderer, VolumeData *volume, int slice_idx, 
                      int coefR, int coefG, int coefB, int slice_direction)
{
    int size = volume->width; // Assuming cubic volume (width=height=depth)
    
    for(int x = 0; x < size; x++) {
        for(int y = 0; y < size; y++) {
            int idx;
            
            // Select slice based on direction (0=XY, 1=XZ, 2=YZ)
            switch(slice_direction) {
                case 0: // XY plane (fixed Z)
                    idx = x + y * size + slice_idx * size * size;
                    break;
                case 1: // XZ plane (fixed Y)
                    idx = x + slice_idx * size + y * size * size;
                    break;
                case 2: // YZ plane (fixed X)
                    idx = slice_idx + y * size + x * size * size;
                    break;
                default:
                    idx = x + y * size + slice_idx * size * size;
            }
            
            if(idx >= 0 && idx < size * size * size) {
                float value = volume->data[idx];
                
                SDL_SetRenderDrawColor(renderer, coefR * value, coefG * value, coefB * value, 255);
                
                // Draw point at appropriate position based on slice direction
                int draw_x, draw_y;
                switch(slice_direction) {
                    case 0: // XY plane
                        draw_x = x;
                        draw_y = y;
                        break;
                    case 1: // XZ plane
                        draw_x = x;
                        draw_y = y;
                        break;
                    case 2: // YZ plane
                        draw_x = y;
                        draw_y = x;
                        break;
                    default:
                        draw_x = x;
                        draw_y = y;
                }
                
                SDL_RenderDrawPoint(renderer, draw_x, draw_y);
            }
        }
    }
}

int main(int argc, char **argv)
{
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <size>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    /**********initialization of SDL elements************/
    int size = atoi(argv[1]);
    SDL_Window *window = NULL;
    SDL_Renderer *rendu = NULL;
    float horiz = 0;
    float vert = 0;
    float depth_offset = 0;  // New parameter for the depth
    float z = 1;
    int coefR = 1;
    int coefG = 1;
    int coefB = 1;
    int tres = 1;
    int current_slice = size / 2;  // Start with middle slice
    int slice_direction = 0;      // 0=XY, 1=XZ, 2=YZ
    
    VolumeData *volume = NULL;
    
    if(SDL_Init(SDL_INIT_VIDEO))
        SDL_Exitwitherror("failed init");
    
    window = SDL_CreateWindow("3D Julia Quaternion", SDL_WINDOWPOS_CENTERED, 0, size, size, 0);            
    if(!window) {
        SDL_Exitwitherror("window creation failed");
    }
    
    rendu = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(!rendu) {
        SDL_DestroyWindow(window);
        SDL_Exitwitherror("renderer failed");
    }
    
    // Generate initial 3D volume
    volume = generateJulia3D(size, horiz, vert, depth_offset, z, tres, Mendel);
    if (!volume) {
        SDL_DestroyRenderer(rendu);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }
    
    SDL_bool Launched = SDL_TRUE;
    SDL_bool needsRedraw = SDL_TRUE;
    SDL_bool needsRecalculation = SDL_TRUE;
    
    while (Launched)
    {
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    Launched = SDL_FALSE;
                    break;
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_ESCAPE:
                            Launched = SDL_FALSE;
                            break;
                        case SDLK_RIGHT:
                            horiz += 10;
                            needsRecalculation = SDL_TRUE;
                            break;
                        case SDLK_LEFT:
                            horiz -= 10;
                            needsRecalculation = SDL_TRUE;
                            break;
                        case SDLK_UP:
                            vert -= 10;
                            needsRecalculation = SDL_TRUE;
                            break;
                        case SDLK_DOWN:
                            vert += 10;
                            needsRecalculation = SDL_TRUE;
                            break;
                        case SDLK_r:
                            coefR += 1;
                            needsRedraw = SDL_TRUE;
                            break;
                        case SDLK_g:
                            coefG += 1;
                            needsRedraw = SDL_TRUE;
                            break;
                        case SDLK_b:
                            coefB += 1;
                            needsRedraw = SDL_TRUE;
                            break;
                        case SDLK_v:
                            coefB -= 1;
                            needsRedraw = SDL_TRUE;
                            break;
                        case SDLK_e:
                            coefR -= 1;
                            needsRedraw = SDL_TRUE;
                            break;
                        case SDLK_f:
                            coefG -= 1;
                            needsRedraw = SDL_TRUE;
                            break;
                        case SDLK_a:
                            z += 0.1;
                            needsRecalculation = SDL_TRUE;
                            break;
                        case SDLK_q:
                            if (z > 0.2) z -= 0.1; // Prevent divide by zero
                            needsRecalculation = SDL_TRUE;
                            break;
                        // Quaternion C parameter adjustments
                        case SDLK_u:
                            C.Iq += (0.01)/z;
                            needsRecalculation = SDL_TRUE;
                            break;
                        case SDLK_j:
                            C.Iq -= (0.01)/z;
                            needsRecalculation = SDL_TRUE;
                            break;
                        case SDLK_i:
                            C.Rq += (0.01)/z;
                            needsRecalculation = SDL_TRUE;
                            break;
                        case SDLK_k:
                            C.Rq -= (0.01)/z;
                            needsRecalculation = SDL_TRUE;
                            break;
                        case SDLK_o:
                            C.Jq += (0.01)/z;
                            needsRecalculation = SDL_TRUE;
                            break;
                        case SDLK_l:
                            C.Jq -= (0.01)/z;
                            needsRecalculation = SDL_TRUE;
                            break;
                        case SDLK_p:
                            C.Kq += (0.01)/z;
                            needsRecalculation = SDL_TRUE;
                            break;
                        case SDLK_m:
                            C.Kq -= (0.01)/z;
                            needsRecalculation = SDL_TRUE;
                            break;
                        case SDLK_w:
                            tres += 1;
                            needsRecalculation = SDL_TRUE;
                            break;
                        case SDLK_x:
                            if (tres > 1) tres -= 1;
                            needsRecalculation = SDL_TRUE;
                            break;
                        // New controls for navigating through slices
                        case SDLK_PAGEUP:
                            current_slice = (current_slice + 1) % size;
                            needsRedraw = SDL_TRUE;
                            break;
                        case SDLK_PAGEDOWN:
                            current_slice = (current_slice + size - 1) % size;
                            needsRedraw = SDL_TRUE;
                            break;
                        // Control for changing slice direction
                        case SDLK_TAB:
                            slice_direction = (slice_direction + 1) % 3;
                            needsRedraw = SDL_TRUE;
                            break;
                        // Control for adjusting depth offset
                        case SDLK_d:
                            depth_offset += 10;
                            needsRecalculation = SDL_TRUE;
                            break;
                        case SDLK_s:
                            depth_offset -= 10;
                            needsRecalculation = SDL_TRUE;
                            break;
                        default:
                            break;
                    }
                default:
                    break;
            }
        }
        
        // Recalculate the 3D volume if necessary
        if (needsRecalculation) {
            // Free previous volume data if it exists
            if (volume) {
                free(volume->data);
                free(volume);
            }
            
            // Generate new 3D volume
            volume = generateJulia3D(size, horiz, vert, depth_offset, z, tres, Mendel);
            if (!volume) {
                fprintf(stderr, "Failed to regenerate volume data\n");
                break;
            }
            
            needsRecalculation = SDL_FALSE;
            needsRedraw = SDL_TRUE;
        }
        
        // Render current slice if necessary
        if (needsRedraw && volume) {
            // Clear renderer
            SDL_SetRenderDrawColor(rendu, 0, 0, 0, 255);
            SDL_RenderClear(rendu);
            
            // Render the current slice
            renderVolumeSlice(rendu, volume, current_slice, coefR, coefG, coefB, slice_direction);
            
            // Display slice information
            char title[100];
            const char* direction_name[3] = {"XY", "XZ", "YZ"};
            sprintf(title, "3D Julia Quaternion - %s Slice %d/%d", 
                    direction_name[slice_direction], current_slice, size);
            SDL_SetWindowTitle(window, title);
            
            // Present renderer
            SDL_RenderPresent(rendu);
            
            needsRedraw = SDL_FALSE;
        }
    }
    
    /*************************************/
    // Clean up
    if (volume) {
        free(volume->data);
        free(volume);
    }
    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
