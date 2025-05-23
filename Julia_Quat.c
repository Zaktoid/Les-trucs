#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <omp.h>
#include <math.h>
#include "Mathutils.h"

quat C={-0.4, 0.6, 0, 0};

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
                            C.Iq += (0.01)/z;
                            break;
                        case SDLK_j:
                            C.Iq -= (0.01)/z;
                            break;
                        case SDLK_i:
                            C.Rq += (0.01)/z;
                            break;
                        case SDLK_k:
                            C.Rq -= (0.01)/z;
                            break;
                        case SDLK_o:
                            C.Jq += (0.01)/z;
                            break;
                        case SDLK_l:
                            C.Jq -= (0.01)/z;
                            break;
                        case SDLK_p:
                            C.Kq += (0.01)/z;
                            break;
                        case SDLK_m:
                            C.Kq -= (0.01)/z; // Changed to 0.01 to match other increments
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
        #pragma omp parallel for collapse(2)
        for(int k = 0; k < hauteur; k++)
        {
            for(int l = 0; l < hauteur; l++)
            {
                quat a = {
                    ((double)k - (hauteur/2) + horiz) / (z * (hauteur/4)),
                    ((double)l - (hauteur/2) + vert) / (z * (hauteur/4)),
                    0,
                    0,
                };
                float s = TestDiverg(5 * z * tres, Mendel, a, 5 * z * tres);
                // Store result in buffer
                result_buffer[k + l * hauteur] = s;
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
