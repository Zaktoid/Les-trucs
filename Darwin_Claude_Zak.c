#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <time.h>
#include <stddef.h>

#define MAX_CELLS 100
#define MAX_FOODSPOTS 200
#define WINDOW_SIZE 400
#define HUNGER_DECREASE_RATE 0.001
#define FOOD_VALUE 0.2
#define CELL_IMPULSE 0.5
#define DRAG_COEFFICIENT 0.95

// Structure for Cell
typedef struct {
    int x;          // Position X
    int y;          // Position Y
    float vx;       // Velocity X
    float vy;       // Velocity Y
    float hunger;   // Hunger value [0,1]
    SDL_bool alive; // Whether the cell is alive
} Cell;

// Structure for FoodSpot
typedef struct {
    int x;          // Position X
    int y;          // Position Y
    float value;    // Nutritional value
    SDL_bool active; // Whether the food spot is active
} FoodSpot;

// Global variables
Cell cells[MAX_CELLS];
FoodSpot foodspots[MAX_FOODSPOTS];
int cell_count = 0;
int food_count = 0;

// Function declarations
void line_food_check(Cell *cell, int prev_x, int prev_y);
void init_cell(int x, int y);
void spawn_random_food(void);
void update_cell(Cell *cell);
void check_food_consumption(Cell *cell);
void handle_key_input(SDL_KeyboardEvent *key);
void render_simulation(SDL_Renderer *renderer);
void SDL_Exitwitherror(const char *message);

// Function to initialize a cell
void init_cell(int x, int y) {
    if (cell_count < MAX_CELLS) {
        cells[cell_count].x = x;
        cells[cell_count].y = y;
        cells[cell_count].vx = 0;
        cells[cell_count].vy = 0;
        cells[cell_count].hunger = 1.0;
        cells[cell_count].alive = SDL_TRUE;
        cell_count++;
        printf("Cell created at (%d, %d)\n", x, y);
    }
}

// Function to spawn a food spot at random location
void spawn_random_food(void) {
    if (food_count < MAX_FOODSPOTS) {
        foodspots[food_count].x = rand() % WINDOW_SIZE;
        foodspots[food_count].y = rand() % WINDOW_SIZE;
        foodspots[food_count].value = FOOD_VALUE;
        foodspots[food_count].active = SDL_TRUE;
        food_count++;
    }
}

// Function to check for food along the path of movement
void line_food_check(Cell *cell, int prev_x, int prev_y) {
    // Use Bresenham's line algorithm to check for food along the movement path
    int dx = abs(cell->x - prev_x);
    int dy = abs(cell->y - prev_y);
    int sx = (prev_x < cell->x) ? 1 : -1;
    int sy = (prev_y < cell->y) ? 1 : -1;
    int err = dx - dy;
    int x = prev_x;
    int y = prev_y;
    
    while (x != cell->x || y != cell->y) {
        // Check for food at this intermediate position
        for (int i = 0; i < food_count; i++) {
            if (foodspots[i].active) {
                int food_dx = x - foodspots[i].x;
                int food_dy = y - foodspots[i].y;
                int dist_sq = food_dx*food_dx + food_dy*food_dy;
                
                if (dist_sq <= 4) { // Within 2 pixel radius
                    printf("LINE CHECK: Food found at (%d, %d) while moving from (%d, %d) to (%d, %d)\n", 
                           foodspots[i].x, foodspots[i].y, prev_x, prev_y, cell->x, cell->y);
                    
                    // Cell consumes food
                    cell->hunger += foodspots[i].value;
                    
                    // Cap hunger at 1.0
                    if (cell->hunger > 1.0) cell->hunger = 1.0;
                    
                    // Deactivate food
                    foodspots[i].active = SDL_FALSE;
                }
            }
        }
        
        // Move to next point on the line
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
}

// Function to update cell state
void update_cell(Cell *cell) {
    if (!cell->alive) return;
    
    // Store previous position for collision check
    int prev_x = cell->x;
    int prev_y = cell->y;
    
    // Apply drag force to gradually slow down the cell
    cell->vx *= DRAG_COEFFICIENT;
    cell->vy *= DRAG_COEFFICIENT;
    
    // If velocity is very small, set it to zero to prevent floating point errors
    if (fabs(cell->vx) < 0.01) cell->vx = 0;
    if (fabs(cell->vy) < 0.01) cell->vy = 0;
    
    // Limit maximum velocity to prevent skipping
    float max_speed = 3.0;
    float speed = sqrt(cell->vx * cell->vx + cell->vy * cell->vy);
    if (speed > max_speed) {
        cell->vx = (cell->vx / speed) * max_speed;
        cell->vy = (cell->vy / speed) * max_speed;
    }
    
    // Update position based on velocity (using float for smoother movement)
    cell->x += (int)cell->vx;
    cell->y += (int)cell->vy;
    
    // Keep cell within boundaries
    if (cell->x < 0) {
        cell->x = 0;
        cell->vx = -cell->vx * 0.5; // Bounce with energy loss
    }
    if (cell->y < 0) {
        cell->y = 0;
        cell->vy = -cell->vy * 0.5; // Bounce with energy loss
    }
    if (cell->x >= WINDOW_SIZE) {
        cell->x = WINDOW_SIZE - 1;
        cell->vx = -cell->vx * 0.5; // Bounce with energy loss
    }
    if (cell->y >= WINDOW_SIZE) {
        cell->y = WINDOW_SIZE - 1;
        cell->vy = -cell->vy * 0.5; // Bounce with energy loss
    }
    
    // Check if cell moved during this update
    if (cell->x != prev_x || cell->y != prev_y) {
        // Check for food along the path of movement to prevent skipping
        line_food_check(cell, prev_x, prev_y);
    }
    
    // Decrease hunger
    cell->hunger -= HUNGER_DECREASE_RATE;
    
    // Check if cell is dead
    if (cell->hunger <= 0) {
        cell->hunger = 0;
        cell->alive = SDL_FALSE;
        printf("Cell died due to starvation\n");
    }
}

// Function to check if a cell can eat food
void check_food_consumption(Cell *cell) {
    if (!cell->alive) return;
    
    for (int i = 0; i < food_count; i++) {
        if (foodspots[i].active) {
            // Distance-based eating: if cell is close enough to food (within 2 pixels), it can eat it
            int dx = cell->x - foodspots[i].x;
            int dy = cell->y - foodspots[i].y;
            int distance_squared = dx*dx + dy*dy;
            
            if (distance_squared <= 4) { // Within 2 pixel radius
                printf("FOOD CONSUMED! Food at (%d, %d), Cell at (%d, %d)\n", 
                       foodspots[i].x, foodspots[i].y, cell->x, cell->y);
                
                // Cell consumes food
                cell->hunger += foodspots[i].value;
                
                // Cap hunger at 1.0
                if (cell->hunger > 1.0) cell->hunger = 1.0;
                
                // Deactivate food
                foodspots[i].active = SDL_FALSE;
            }
        }
    }
}

// Function to handle keyboard input for controlling cells
void handle_key_input(SDL_KeyboardEvent *key) {
    // Control the first cell for now
    if (cell_count == 0) return;
    
    Cell *cell = &cells[0];
    
    switch (key->keysym.sym) {
        case SDLK_UP:
            cell->vy -= CELL_IMPULSE; // Apply impulse (bump) upward
            break;
        case SDLK_DOWN:
            cell->vy += CELL_IMPULSE; // Apply impulse (bump) downward
            break;
        case SDLK_LEFT:
            cell->vx -= CELL_IMPULSE; // Apply impulse (bump) left
            break;
        case SDLK_RIGHT:
            cell->vx += CELL_IMPULSE; // Apply impulse (bump) right
            break;
        case SDLK_SPACE:
            // Immediate stop (emergency brake)
            cell->vx = 0;
            cell->vy = 0;
            break;
        case SDLK_f:
            // Spawn new food
            spawn_random_food();
            printf("New food spawned\n");
            break;
        case SDLK_c:
            // Create new cell at random position
            init_cell(rand() % WINDOW_SIZE, rand() % WINDOW_SIZE);
            break;
        default:
            break;
    }
}

// Function to render all simulation elements
void render_simulation(SDL_Renderer *renderer) {
    // Clear the screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    // Render food spots - making them bigger for better visibility
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);  // Green
    for (int i = 0; i < food_count; i++) {
        if (foodspots[i].active) {
            // Draw a 2x2 pixel food for better visibility
            SDL_Rect foodRect = {foodspots[i].x, foodspots[i].y, 2, 2};
            SDL_RenderFillRect(renderer, &foodRect);
        }
    }
    
    // Render cells - making them bigger for better visibility
    for (int i = 0; i < cell_count; i++) {
        if (cells[i].alive) {
            // Color based on hunger (red when hungry, blue when full)
            int red = (int)(255 * (1 - cells[i].hunger));
            int blue = (int)(255 * cells[i].hunger);
            SDL_SetRenderDrawColor(renderer, red, 0, blue, 255);
            
            // Draw a 3x3 pixel cell for better visibility
            SDL_Rect cellRect = {cells[i].x - 1, cells[i].y - 1, 3, 3};
            SDL_RenderFillRect(renderer, &cellRect);
        }
    }
    
    // Render debug information
    char debug_info[100];
    if (cell_count > 0) {
        sprintf(debug_info, "Hunger: %.2f", cells[0].hunger);
        // Print to console instead
        printf("%s\n", debug_info);
    }
}

void SDL_Exitwitherror(const char *message) {
    SDL_Log("Erreur:%s >%s \n", message, SDL_GetError());
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    srand(time(NULL));
    int hauteur = WINDOW_SIZE;
    SDL_Window *window;
    SDL_Renderer *rendu;
    
    if (SDL_Init(SDL_INIT_VIDEO))
        SDL_Exitwitherror("failed init");
        
    window = SDL_CreateWindow("Darwinian Evolution Simulation", 
                             SDL_WINDOWPOS_CENTERED, 0, 
                             hauteur, hauteur, 0);
                             
    if (!window)
        SDL_Exitwitherror("window creation failed");
        
    rendu = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    
    if (!rendu)
        SDL_Exitwitherror("renderer failed");
        
    // Initialize simulation with one cell and some food
    init_cell(WINDOW_SIZE / 2, WINDOW_SIZE / 2);
    
    // Create some initial food scattered around the cell
    for (int i = 0; i < 100; i++) {
        spawn_random_food();
    }
    
    // Print debugging instructions
    printf("Debugging enabled. Watch for 'FOOD CONSUMED!' messages.\n");
    printf("Controls: Arrow keys to move, Space to stop, F to spawn food, C to create new cell.\n");
    
    SDL_bool Launched = SDL_TRUE;
    Uint32 previous_time = SDL_GetTicks();
    Uint32 lag = 0;
    const Uint32 MS_PER_UPDATE = 16;  // ~60 updates per second
    
    while (Launched) {
        Uint32 current_time = SDL_GetTicks();
        Uint32 elapsed = current_time - previous_time;
        previous_time = current_time;
        lag += elapsed;
        
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    Launched = SDL_FALSE;
                    break;
                case SDL_KEYDOWN:
                    handle_key_input(&event.key);
                    break;
                default:
                    break;
            }
        }
        
        // Update game state at fixed intervals
        while (lag >= MS_PER_UPDATE) {
            // Update all cells
            for (int i = 0; i < cell_count; i++) {
                update_cell(&cells[i]);
                check_food_consumption(&cells[i]);
            }
            
            lag -= MS_PER_UPDATE;
        }
        
        // Render the simulation
        render_simulation(rendu);
        SDL_RenderPresent(rendu);
        
        // Add a small delay to prevent maxing out CPU
        SDL_Delay(1);
    }

    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}