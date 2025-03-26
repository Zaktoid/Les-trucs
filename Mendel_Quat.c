#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <omp.h>
#include <math.h>
#include "Mathutils.h"

quat START={0, 0, 0, 0};

void SDL_Exitwitherror(const char *message)
{
    SDL_Log("Erreur:%s >%s \n", message, SDL_GetError());
    exit(EXIT_FAILURE);
}

inline float TestDiverg(int tres, quat a, quat C, double toler)
{
    quat interation = a;
    const double toler_squared = toler * toler; // Compare squared modules to avoid sqrt
    
    for(int k = 0; k < tres; k++)
    {
        // Perform quaternion operations directly to avoid function call overhead
        // This assumes you know the implementation of Pow_q and sum_q
        // Replace with the actual implementation from Mathutils.c
        
        // Pow_q(interation, 2) inline expansion:
        quat squared = {
            interation.Rq * interation.Rq - interation.Iq * interation.Iq - 
            interation.Jq * interation.Jq - interation.Kq * interation.Kq,
            2 * interation.Rq * interation.Iq,
            2 * interation.Rq * interation.Jq,
            2 * interation.Rq * interation.Kq
        };
        
        // sum_q inline expansion:
        interation.Rq = squared.Rq + C.Rq;
        interation.Iq = squared.Iq + C.Iq;
        interation.Jq = squared.Jq + C.Jq;
        interation.Kq = squared.Kq + C.Kq;
        
        // Squared module check (faster than sqrt)
        double mod_squared = interation.Rq * interation.Rq + interation.Iq * interation.Iq + 
                            interation.Jq * interation.Jq + interation.Kq * interation.Kq;
        
        if(mod_squared > toler_squared)
            return k;
    }
    return 0;
}



int main(int argc, char **argv)
{
    /**********intialisation d'élements SDL************/
    int hauteur = atoi(argv[1]);
    SDL_Window *window = NULL;
    SDL_Renderer *rendu = NULL;
    float horiz = 0;
    float vert = 0;
    float z = 1;
    int coefR = 1;
    int coefG = 1;
    int coefB = 1;
    int tres = 1;
    
    // Create a buffer to store calculation results
    float *result_buffer = (float*)malloc(hauteur * hauteur * sizeof(float));
    if (!result_buffer) {
        fprintf(stderr, "Failed to allocate memory for result buffer\n");
        return EXIT_FAILURE;
    }
    
    if(SDL_Init(SDL_INIT_VIDEO))
        SDL_Exitwitherror("failed init");
    
    window = SDL_CreateWindow("Julia Quaternion", SDL_WINDOWPOS_CENTERED, 0, hauteur, hauteur, 0);            
    if(!window) {
        free(result_buffer);
        SDL_Exitwitherror("window creation failed");
    }
    
    rendu = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(!rendu) {
        SDL_DestroyWindow(window);
        free(result_buffer);
        SDL_Exitwitherror("renderer failed");
    }
    
    SDL_bool Launched = SDL_TRUE;
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
                            break;
                        case SDLK_LEFT:
                            horiz -= 10;
                            break;
                        case SDLK_UP:
                            vert -= 10;
                            break;
                        case SDLK_DOWN:
                            vert += 10;
                            break;
                        case SDLK_r:
                            coefR += 1;
                            break;
                        case SDLK_g:
                            coefG += 1;
                            break;
                        case SDLK_b:
                            coefB += 1;
                            break;
                        case SDLK_v:
                            coefB -= 1;
                            break;
                        case SDLK_e:
                            coefR -= 1;
                            break;
                        case SDLK_f:
                            coefG -= 1;
                            break;
                        case SDLK_a:
                            z += 0.1;
                            break;
                        case SDLK_q:
                            if (z > 0.2) z -= 0.1; // Prevent divide by zero
                            break;
                        case SDLK_u:
                            START.Iq += (0.01)/z;
                            break;
                        case SDLK_j:
                            START.Iq -= (0.01)/z;
                            break;
                        case SDLK_i:
                            START.Rq += (0.01)/z;
                            break;
                        case SDLK_k:
                            START.Rq -= (0.01)/z;
                            break;
                        case SDLK_o:
                            START.Jq += (0.01)/z;
                            break;
                        case SDLK_l:
                            START.Jq -= (0.01)/z;
                            break;
                        case SDLK_p:
                            START.Kq += (0.01)/z;
                            break;
                        case SDLK_m:
                            START.Kq -= (0.01)/z; // Changed to 0.01 to match other increments
                            break;
                        case SDLK_w:
                            tres += 1;
                            break;
                        case SDLK_x:
                            if (tres > 1) tres -= 1; // Using integer decrement
                            break;
                        default:
                            break;
                    }
                default:
                    break;
            }
        }
        
        // Calculate Julia set in parallel and store in buffer
        const int CHUNK_SIZE = 32; // Adjust based on your cache size

        #pragma omp parallel for collapse(2) schedule(dynamic)
        for(int y_chunk = 0; y_chunk < hauteur; y_chunk += CHUNK_SIZE) {
        for(int x_chunk = 0; x_chunk < hauteur; x_chunk += CHUNK_SIZE) {
            // Process a chunk
            int y_max = y_chunk + CHUNK_SIZE > hauteur ? hauteur : y_chunk + CHUNK_SIZE;
            int x_max = x_chunk + CHUNK_SIZE > hauteur ? hauteur : x_chunk + CHUNK_SIZE;
            
            for(int l = y_chunk; l < y_max; l++) {
                for(int k = x_chunk; k < x_max; k++) {
                    quat C = {
                        ((double)k - (hauteur/2) + horiz) / (z * (hauteur/4)),
                        ((double)l - (hauteur/2) + vert) / (z * (hauteur/4)),
                        0,
                        0,
                    };
                    float s = TestDiverg(5 * z * tres, START, C, 5 * z * tres);
                    // Store in row-major order for better cache hits
                    result_buffer[l * hauteur + k] = s;
                }
            }
        }
        }
        
        // Render results sequentially (thread-safe)
        for(int k = 0; k < hauteur; k++)
        {
            for(int l = 0; l < hauteur; l++)
            {
                float s = result_buffer[k + l * hauteur];
                if(SDL_SetRenderDrawColor(rendu, (coefR) * s, (coefG) * s, (coefB) * s, 255) != 0) {
                    fprintf(stderr, "SetRenderDrawColor failed: %s\n", SDL_GetError());
                    continue;
                }
                if(SDL_RenderDrawPoint(rendu, k, l) != 0) {
                    fprintf(stderr, "RenderDrawPoint failed: %s\n", SDL_GetError());
                }
            }
        }
        
        SDL_RenderPresent(rendu);
    }
    
    /*************************************/
    free(result_buffer);
    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
