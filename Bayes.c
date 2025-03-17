#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>

#define MAX_N 50
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MARGIN 50
#define BAR_SPACING 2
#define TEXT_MARGIN 10

// Global variables for Bayesian parameters
int N = 10;              // Number of bins
int sample_size = 20;    // Sample size n
double proportion = 0.25; // y/n ratio
double *probabilities = NULL;
double *prior = NULL;
int iteration = 0;
TTF_Font *font = NULL;

void SDL_ExitWithError(const char *message) {
    SDL_Log("Error: %s > %s\n", message, SDL_GetError());
    exit(EXIT_FAILURE);
}

// Function to render text
void render_text(SDL_Renderer *renderer, const char *text, int x, int y, SDL_Color color) {
    SDL_Surface *surface = TTF_RenderText_Blended(font, text, color);
    if (!surface) {
        printf("TTF_RenderText_Blended error: %s\n", TTF_GetError());
        return;
    }
    
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("SDL_CreateTextureFromSurface error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    
    SDL_Rect rect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

// Binomial coefficient calculation
double binomial_coeff(int n, int k) {
    if (k < 0 || k > n) return 0;
    if (k == 0 || k == n) return 1;
    double coeff = 1;
    for (int i = 0; i < k; i++) {
        coeff *= (n - i);
        coeff /= (i + 1);
    }
    return coeff;
}

// Update probabilities
void update_probabilities() {
    if (!probabilities) probabilities = malloc(N * sizeof(double));
    
    int y = round(sample_size * proportion);
    double sum = 0.0;
    
    // Calculate unnormalized probabilities
    for (int i = 0; i < N; i++) {
        double theta = (i + 1.0) / N;
        double prior_prob = prior ? prior[i] : 1.0 / N;
        double likelihood = binomial_coeff(sample_size, y) * 
                          pow(theta, y) * 
                          pow(1 - theta, sample_size - y);
        probabilities[i] = prior_prob * likelihood;
        sum += probabilities[i];
    }
    
    // Normalize
    for (int i = 0; i < N; i++) {
        probabilities[i] /= sum;
    }
}

// Draw the visualization
void draw_visualization(SDL_Renderer *renderer) {
    // Clear screen
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    
    // Draw axes
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawLine(renderer, MARGIN, WINDOW_HEIGHT - MARGIN, 
                      WINDOW_WIDTH - MARGIN, WINDOW_HEIGHT - MARGIN); // X axis
    SDL_RenderDrawLine(renderer, MARGIN, MARGIN, 
                      MARGIN, WINDOW_HEIGHT - MARGIN); // Y axis
    
    // Draw bars
    int bar_width = (WINDOW_WIDTH - 2 * MARGIN) / N - BAR_SPACING;
    double max_prob = 0;
    for (int i = 0; i < N; i++) {
        if (probabilities[i] > max_prob) max_prob = probabilities[i];
    }
    
    for (int i = 0; i < N; i++) {
        SDL_Rect bar = {
            MARGIN + i * (bar_width + BAR_SPACING),
            WINDOW_HEIGHT - MARGIN - (int)((WINDOW_HEIGHT - 2 * MARGIN) * probabilities[i] / max_prob),
            bar_width,
            (int)((WINDOW_HEIGHT - 2 * MARGIN) * probabilities[i] / max_prob)
        };
        SDL_SetRenderDrawColor(renderer, 136, 132, 216, 255); // Purple color
        SDL_RenderFillRect(renderer, &bar);
        
        // Draw theta values under bars
        char theta_text[10];
        snprintf(theta_text, sizeof(theta_text), "%.2f", (i + 1.0) / N);
        SDL_Color black = {0, 0, 0, 255};
        render_text(renderer, theta_text, 
                   MARGIN + i * (bar_width + BAR_SPACING), 
                   WINDOW_HEIGHT - MARGIN + TEXT_MARGIN, black);
    }
    
    // Render parameters
    SDL_Color black = {0, 0, 0, 255};
    char text_buffer[100];
    
    // Title
    snprintf(text_buffer, sizeof(text_buffer), "Bayesian Update Visualization - Iteration %d", iteration);
    render_text(renderer, text_buffer, TEXT_MARGIN, TEXT_MARGIN, black);
    
    // Parameters
    snprintf(text_buffer, sizeof(text_buffer), "N = %d", N);
    render_text(renderer, text_buffer, TEXT_MARGIN, TEXT_MARGIN + 30, black);
    
    snprintf(text_buffer, sizeof(text_buffer), "Sample size (n) = %d", sample_size);
    render_text(renderer, text_buffer, TEXT_MARGIN, TEXT_MARGIN + 50, black);
    
    snprintf(text_buffer, sizeof(text_buffer), "Proportion (y/n) = %.3f", proportion);
    render_text(renderer, text_buffer, TEXT_MARGIN, TEXT_MARGIN + 70, black);
    
    snprintf(text_buffer, sizeof(text_buffer), "Successes (y) = %d", (int)(sample_size * proportion));
    render_text(renderer, text_buffer, TEXT_MARGIN, TEXT_MARGIN + 90, black);
    
    // Controls
    render_text(renderer, "Controls:", TEXT_MARGIN, WINDOW_HEIGHT - 120, black);
    render_text(renderer, "Arrow Up/Down: Change sample size", TEXT_MARGIN, WINDOW_HEIGHT - 100, black);
    render_text(renderer, "Arrow Left/Right: Change N", TEXT_MARGIN, WINDOW_HEIGHT - 80, black);
    render_text(renderer, "Q/E: Change proportion", TEXT_MARGIN, WINDOW_HEIGHT - 60, black);
    render_text(renderer, "Space: Next iteration", TEXT_MARGIN, WINDOW_HEIGHT - 40, black);
    render_text(renderer, "R: Reset", TEXT_MARGIN, WINDOW_HEIGHT - 20, black);
    
    SDL_RenderPresent(renderer);
}

// Handle user input
void handle_keypress(SDL_Keycode key) {
    switch (key) {
        case SDLK_UP:
            sample_size = fmin(sample_size + 5, 1000);
            update_probabilities();
            break;
        case SDLK_DOWN:
            sample_size = fmax(sample_size - 5, 1);
            update_probabilities();
            break;
        case SDLK_LEFT:
            N = fmax(N - 1, 2);
            free(probabilities);
            probabilities = NULL;
            free(prior);
            prior = NULL;
            iteration = 0;
            update_probabilities();
            break;
        case SDLK_RIGHT:
            N = fmin(N + 1, MAX_N);
            free(probabilities);
            probabilities = NULL;
            free(prior);
            prior = NULL;
            iteration = 0;
            update_probabilities();
            break;
        case SDLK_q:
            proportion = fmax(proportion - 0.05, 0.0);
            update_probabilities();
            break;
        case SDLK_e:
            proportion = fmin(proportion + 0.05, 1.0);
            update_probabilities();
            break;
        case SDLK_SPACE:
            // Store current probabilities as prior for next iteration
            if (!prior) prior = malloc(N * sizeof(double));
            for (int i = 0; i < N; i++) {
                prior[i] = probabilities[i];
            }
            iteration++;
            update_probabilities();
            break;
        case SDLK_r:
            // Reset to uniform prior
            free(prior);
            prior = NULL;
            iteration = 0;
            update_probabilities();
            break;
    }
}

int main(int argc, char **argv) {
    SDL_Window *window;
    SDL_Renderer *renderer;
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        SDL_ExitWithError("SDL initialization failed");
        
    if (TTF_Init() < 0) {
        printf("TTF_Init error: %s\n", TTF_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }
    
    // Load font
    font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 12);
    if (!font) {
        printf("TTF_OpenFont error: %s\n", TTF_GetError());
        TTF_Quit();
        SDL_Quit();
        return EXIT_FAILURE;
    }
        
    window = SDL_CreateWindow("Bayesian Update Visualization",
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!window)
        SDL_ExitWithError("Window creation failed");
        
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer)
        SDL_ExitWithError("Renderer creation failed");
    
    // Initialize probabilities
    update_probabilities();
    
    SDL_bool running = SDL_TRUE;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = SDL_FALSE;
                    break;
                case SDL_KEYDOWN:
                    handle_keypress(event.key.keysym.sym);
                    break;
            }
        }
        
        draw_visualization(renderer);
        SDL_Delay(16); // Cap at ~60 FPS
    }
    
    // Cleanup
    free(probabilities);
    free(prior);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return EXIT_SUCCESS;
}