#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <omp.h>    // OpenMP header
#include "Mathutils.h"

#define MAX_RADIUS 1000000  // Maximum radius to compute

void SDL_Exitwitherror(const char *message)
{
    SDL_Log("Erreur:%s >%s \n", message, SDL_GetError());
    exit(EXIT_FAILURE);
}

// Calculate the number of prime factors for a number
int countPrimeFactors(int n) {
    if (n <= 1) return 0;
    
    int count = 0;
    
    // Check for divisibility by 2
    while (n % 2 == 0) {
        count++;
        n /= 2;
    }
    
    // Check for divisibility by odd numbers starting from 3
    for (int i = 3; i * i <= n; i += 2) {
        while (n % i == 0) {
            count++;
            n /= i;
        }
    }
    
    // If n is a prime number greater than 2
    if (n > 2) {
        count++;
    }
    
    return count;
}

// For optimization: precompute prime factors up to a certain limit
void precomputePrimeFactorCounts(int *factorCounts, int limit) {
    #pragma omp parallel for
    for (int i = 1; i <= limit; i++) {
        factorCounts[i] = countPrimeFactors(i);
    }
}

// Render based on "primeness" (number of prime factors)
// Higher values = less "prime-like"
void renderPrimeness(SDL_Renderer *rendu, int hauteur, complex_num C, int zoom, int maxRadius) {
    int *factorCounts = (int *)malloc((maxRadius + 1) * sizeof(int));
    
    // Precompute prime factor counts
    printf("Precomputing prime factor counts up to %d...\n", maxRadius);
    precomputePrimeFactorCounts(factorCounts, maxRadius);
    
    // Create texture for rendering
    SDL_Texture *texture = SDL_CreateTexture(rendu, 
                                       SDL_PIXELFORMAT_ABGR8888, 
                                       SDL_TEXTUREACCESS_STREAMING, 
                                       hauteur, hauteur);
    
    Uint32 *pixels = (Uint32 *)malloc(hauteur * hauteur * sizeof(Uint32));
    memset(pixels, 0, hauteur * hauteur * sizeof(Uint32));
    
    // Maximum observed prime factor count for normalization
    int maxFactors = 0;
    for (int i = 1; i <= maxRadius; i++) {
        if (factorCounts[i] > maxFactors) {
            maxFactors = factorCounts[i];
        }
    }
    
    printf("Maximum prime factors found: %d\n", maxFactors);
    
    // Constants for coloring
    const float gamma = 0.7f;
    
    // Compute colors for all points in the circular field
    #pragma omp parallel for
    for (int r = 1; r <= maxRadius; r++) {
        for (int theta = 0; theta < 360; theta++) {
            float angle = (float)theta * M_PI / 180.0f;
            
            // Use polar coordinates to determine position
            complex_num cis = {cos(angle), sin(angle)};
            complex_num R = {r / (float)zoom, 0};
            complex_num P = sum(prod(R, cis), C);
            
            int x = (int)P.Rz;
            int y = (int)P.Iz;
            
            // Check if point is within the window
            if (x >= 0 && x < hauteur && y >= 0 && y < hauteur) {
                // Get primeness (number of factors)
                int factors = factorCounts[r];
                
                // Prime numbers have exactly 1 factor, so invert the scale
                // 1 factor (prime) = high primeness = 1.0
                // Many factors = low primeness = approaching 0.0
                float primeness;
                if (factors == 1) {
                    // Special case for primes - brightest
                    primeness = 1.0f;
                } else {
                    // Inverse relationship - fewer factors means more "prime-like"
                    primeness = 1.0f - ((float)(factors - 1) / maxFactors);
                }
                
                // Apply gamma correction
                float correctedValue = powf(primeness, gamma);
                
                // Color based on primeness - from blue (composite) to white (prime)
                Uint8 r, g, b;
                
                if (correctedValue < 0.25f) {
                    // Deep blue to purple (least prime-like)
                    r = (Uint8)(correctedValue * 4 * 128);
                    g = 0;
                    b = 255;
                } else if (correctedValue < 0.5f) {
                    // Purple to red
                    float t = (correctedValue - 0.25f) * 4.0f;
                    r = (Uint8)(128 + t * 127);
                    g = 0;
                    b = (Uint8)(255 * (1.0f - t));
                } else if (correctedValue < 0.75f) {
                    // Red to yellow
                    float t = (correctedValue - 0.5f) * 4.0f;
                    r = 255;
                    g = (Uint8)(t * 255);
                    b = 0;
                } else {
                    // Yellow to white (prime numbers)
                    float t = (correctedValue - 0.75f) * 4.0f;
                    r = 255;
                    g = 255;
                    b = (Uint8)(t * 255);
                }
                
                // Set pixel color with full alpha
                int idx = y * hauteur + x;
                pixels[idx] = (255 << 24) | (b << 16) | (g << 8) | r;
            }
        }
    }
    
    // Update texture and render
    SDL_UpdateTexture(texture, NULL, pixels, hauteur * sizeof(Uint32));
    SDL_RenderCopy(rendu, texture, NULL, NULL);
    
    // Clean up
    SDL_DestroyTexture(texture);
    free(pixels);
    free(factorCounts);
}

int main()
{
    srand(time(NULL));
    /**********intialisation d'élements SDL************/
    int hauteur = 800;
    SDL_Window *window;
    SDL_Renderer *rendu;
    int zoom = 1;
    
    printf("PrimeGalaxy - Visualizing primeness\n");
    printf("OpenMP using %d threads\n", omp_get_max_threads());
    
    if (SDL_Init(SDL_INIT_VIDEO))
        SDL_Exitwitherror("failed init");
        
    window = SDL_CreateWindow("Prime Galaxy - Primeness Visualization", SDL_WINDOWPOS_CENTERED, 0, hauteur, hauteur, 0);
    if (!window)
        SDL_Exitwitherror("window creation failed");
        
    rendu = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!rendu)
        SDL_Exitwitherror("renderer failed");
    
    // Use hardware acceleration and vsync
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    
    complex_num C = {hauteur / 2, hauteur / 2};
    bool needsRecalculation = true;
    
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
                        case SDLK_z:
                            // Adaptive zoom in - increase more at higher zoom levels
                            if (zoom < 10)
                                zoom += 1;
                            else if (zoom < 50)
                                zoom += 5;
                            else if (zoom < 200)
                                zoom += 20;
                            else
                                zoom += zoom / 10; // 10% increase
                            
                            needsRecalculation = true;
                            break;
                        case SDLK_a:
                            // Adaptive zoom out - decrease more at higher zoom levels
                            if (zoom <= 10)
                            {
                                if (zoom > 1)
                                    zoom -= 1;
                            }
                            else if (zoom <= 50)
                                zoom -= 5;
                            else if (zoom <= 200)
                                zoom -= 20;
                            else
                                zoom -= zoom / 10; // 10% decrease
                            
                            // Ensure we don't go below 1
                            if (zoom < 1)
                                zoom = 1;
                                
                            needsRecalculation = true;
                            break;
                        default:
                            break;
                    }
                default:
                    break;
            }
        }
        
        if (needsRecalculation)
        {
            // Clear screen
            SDL_SetRenderDrawColor(rendu, 0, 0, 0, 255);
            SDL_RenderClear(rendu);
            
            // Render primeness visualization
            renderPrimeness(rendu, hauteur, C, zoom, MAX_RADIUS);
            
            SDL_RenderPresent(rendu);
            needsRecalculation = false;
            
            printf("Rendered with zoom level: %d\n", zoom);
        }
    }

    /*************************************/
    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}