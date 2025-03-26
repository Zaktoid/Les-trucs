#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <time.h>
#include <string.h>

#define MAX_SEQUENCE_SIZE 40000  // Maximum size for sequence arrays
#define MAX_TERMS 100  // Maximum number of terms to display
#define CELL_SIZE 8   // Size of each cell in pixels
#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800

// Define different colors for each digit (0-9)
SDL_Color digitColors[10] = {
    {255, 255, 255, 255},  // 0: White
    {255, 0, 0, 255},      // 1: Red
    {0, 255, 0, 255},      // 2: Green
    {0, 0, 255, 255},      // 3: Blue
    {255, 255, 0, 255},    // 4: Yellow
    {255, 0, 255, 255},    // 5: Magenta
    {0, 255, 255, 255},    // 6: Cyan
    {128, 128, 128, 255},  // 7: Gray
    {255, 128, 0, 255},    // 8: Orange
    {128, 0, 255, 255}     // 9: Purple
};

void SDL_ExitWithError(const char *message)
{
    SDL_Log("Error: %s > %s\n", message, SDL_GetError());
    exit(EXIT_FAILURE);
}

// Function to compute the next term in the look-and-say sequence
// Implementation of the LookAndSay function from the template
int LookAndSay(int* in, int size, int* out)
{
    if (size <= 0) return 0;
    
    int outIndex = 0;
    int currentDigit = in[0];
    int count = 1;
    
    for (int i = 1; i < size; i++) {
        if (in[i] == currentDigit) {
            count++;
        } else {
            // Write count and digit to output
            out[outIndex++] = count;
            out[outIndex++] = currentDigit;
            
            // Reset for next digit
            currentDigit = in[i];
            count = 1;
        }
    }
    
    // Handle the last group
    out[outIndex++] = count;
    out[outIndex++] = currentDigit;
    
    return outIndex;
}

// Draw the entire sequence pyramid with each term on a new row
void DrawSequencePyramid(SDL_Renderer* renderer, int sequences[][MAX_SEQUENCE_SIZE], 
                         int sequenceSizes[], int termCount, int startTerm,
                         int offsetX, int offsetY, float zoom)
{
    int cellSize = (int)(CELL_SIZE * zoom);
    
    // Calculate how many terms can fit on screen with current zoom
    int visibleTerms = fmin(termCount - startTerm, WINDOW_HEIGHT / cellSize);
    
    for (int t = 0; t < visibleTerms; t++) {
        int term = startTerm + t;
        int termLength = sequenceSizes[term];
        
        // Center the term horizontally
        int rowStartX = offsetX + (WINDOW_WIDTH - termLength * cellSize) / 2;
        int rowY = offsetY + t * cellSize;
        
        // Draw each digit in this term
        for (int i = 0; i < termLength; i++) {
            int digit = sequences[term][i];
            
            // Set color based on digit
            SDL_SetRenderDrawColor(renderer, 
                                 digitColors[digit].r,
                                 digitColors[digit].g,
                                 digitColors[digit].b,
                                 digitColors[digit].a);
            
            // Draw cell
            SDL_Rect cellRect = {rowStartX + i * cellSize, rowY, cellSize, cellSize};
            SDL_RenderFillRect(renderer, &cellRect);
            
            // Draw border (optional)
            if (cellSize > 3) {
                SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);  // Dark border
                SDL_RenderDrawRect(renderer, &cellRect);
            }
        }
    }
    
    // Display information about current view
    printf("Showing terms %d to %d, Zoom: %.2f\n", 
           startTerm + 1, startTerm + visibleTerms, zoom);
}

int main(int argc, char **argv)
{
    srand(time(NULL));
    SDL_Window *window;
    SDL_Renderer *rendu;
    
    if(SDL_Init(SDL_INIT_VIDEO))
        SDL_ExitWithError("Failed init");
    
    window = SDL_CreateWindow("Look-and-Say Sequence Pyramid", 
                             SDL_WINDOWPOS_CENTERED, 
                             SDL_WINDOWPOS_CENTERED, 
                             WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if(!window)
        SDL_ExitWithError("Window creation failed");
    
    rendu = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(!rendu)
        SDL_ExitWithError("Renderer failed");
    
    // Initialize the first term of the sequence with just '1'
    int sequences[MAX_TERMS][MAX_SEQUENCE_SIZE];
    int sequenceSizes[MAX_TERMS];
    
    sequences[0][0] = 1;  // Start with "1"
    sequenceSizes[0] = 1;
    
    // Generate all terms of the sequence
    int termCount = 1;
    for (int i = 1; i < MAX_TERMS; i++) {
        sequenceSizes[i] = LookAndSay(sequences[i-1], sequenceSizes[i-1], sequences[i]);
        termCount++;
        
        // Print the current term number being generated
        printf("Generated term %d, length: %d\n", i+1, sequenceSizes[i]);
        
        // Stop if the sequence gets too large
        if (sequenceSizes[i] > MAX_SEQUENCE_SIZE/2) {
            printf("Reached maximum sequence size at term %d\n", i+1);
            break;
        }
    }
    
    SDL_bool Launched = SDL_TRUE;
    int startTerm = 0;        // First term to display
    float zoom = 1.0f;        // Zoom level
    int offsetX = 0;          // Horizontal offset for panning
    int offsetY = 10;         // Vertical offset for panning
    
    // Display instructions
    printf("\nLook-and-Say Sequence Pyramid Visualizer\n");
    printf("----------------------------------------\n");
    printf("Generated %d terms of the sequence\n", termCount);
    printf("Controls:\n");
    printf("  UP/DOWN: Scroll through terms\n");
    printf("  +/-: Zoom in/out\n");
    printf("  ARROW KEYS: Pan the view\n");
    printf("  R: Reset view\n");
    printf("  PAGE UP/DOWN: Jump 10 terms at a time\n");
    printf("  HOME/END: Jump to first/last term\n\n");
    
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
                        case SDLK_UP:
                            // Scroll up
                            if (startTerm > 0) startTerm--;
                            break;
                        case SDLK_DOWN:
                            // Scroll down
                            if (startTerm < termCount - 1) startTerm++;
                            break;
                        case SDLK_PLUS:
                        case SDLK_EQUALS:
                            // Zoom in
                            zoom *= 1.2f;
                            if (zoom > 10.0f) zoom = 10.0f;
                            break;
                        case SDLK_MINUS:
                            // Zoom out
                            zoom /= 1.2f;
                            if (zoom < 0.2f) zoom = 0.2f;
                            break;
                        case SDLK_LEFT:
                            // Pan left
                            offsetX += 20;
                            break;
                        case SDLK_RIGHT:
                            // Pan right
                            offsetX -= 20;
                            break;
                        case SDLK_r:
                            // Reset view
                            startTerm = 0;
                            zoom = 1.0f;
                            offsetX = 0;
                            offsetY = 10;
                            break;
                        case SDLK_PAGEUP:
                            // Jump 10 terms up
                            startTerm = (startTerm > 10) ? startTerm - 10 : 0;
                            break;
                        case SDLK_PAGEDOWN:
                            // Jump 10 terms down
                            startTerm = (startTerm + 10 < termCount) ? startTerm + 10 : termCount - 1;
                            break;
                        case SDLK_HOME:
                            // Jump to first term
                            startTerm = 0;
                            break;
                        case SDLK_END:
                            // Jump to last term
                            startTerm = termCount - 1;
                            break;
                        default:
                            break;
                    }
                    break;
                case SDL_MOUSEWHEEL:
                    // Use mouse wheel to zoom
                    if (event.wheel.y > 0) {
                        // Zoom in
                        zoom *= 1.1f;
                        if (zoom > 10.0f) zoom = 10.0f;
                    } else if (event.wheel.y < 0) {
                        // Zoom out
                        zoom /= 1.1f;
                        if (zoom < 0.2f) zoom = 0.2f;
                    }
                    break;
                default:
                    break;
            }
        }
        
        // Clear screen
        SDL_SetRenderDrawColor(rendu, 20, 20, 50, 255);  // Dark blue background
        SDL_RenderClear(rendu);
        
        // Draw the sequence pyramid
        DrawSequencePyramid(rendu, sequences, sequenceSizes, termCount, startTerm, offsetX, offsetY, zoom);
        
        SDL_RenderPresent(rendu);
        SDL_Delay(20);  // Small delay to prevent maxing CPU
    }
    
    // Cleanup
    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}