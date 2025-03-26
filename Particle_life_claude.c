#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <time.h>
#include <omp.h> // Add OpenMP for multithreading

// Grid cell structure for spatial partitioning
typedef struct {
    int* particles;          // Array of particle indices in this cell
    int count;               // Number of particles in this cell
    int capacity;            // Capacity of the particles array
} GridCell;

// Structure of Arrays (SoA) for better cache utilization and vectorization
typedef struct {
    float* pos_x;            // X positions
    float* pos_y;            // Y positions
    float* vel_x;            // X velocities
    float* vel_y;            // Y velocities
    float* force_x;          // X forces
    float* force_y;          // Y forces
    int* colors;             // Colors
    int count;               // Particle count
} ParticleSystem;

// Grid for spatial partitioning
typedef struct {
    GridCell* cells;         // Grid cells
    int width;               // Grid width
    int height;              // Grid height
    float cell_size;         // Size of each cell
} Grid;

// Create particle system
ParticleSystem* CreateParticleSystem(int count) {
    ParticleSystem* ps = malloc(sizeof(ParticleSystem));
    if (!ps) return NULL;
    
    ps->pos_x = malloc(sizeof(float) * count);
    ps->pos_y = malloc(sizeof(float) * count);
    ps->vel_x = malloc(sizeof(float) * count);
    ps->vel_y = malloc(sizeof(float) * count);
    ps->force_x = malloc(sizeof(float) * count);
    ps->force_y = malloc(sizeof(float) * count);
    ps->colors = malloc(sizeof(int) * count);
    ps->count = count;
    
    if (!ps->pos_x || !ps->pos_y || !ps->vel_x || !ps->vel_y || 
        !ps->force_x || !ps->force_y || !ps->colors) {
        // Handle allocation failure
        free(ps->pos_x);
        free(ps->pos_y);
        free(ps->vel_x);
        free(ps->vel_y);
        free(ps->force_x);
        free(ps->force_y);
        free(ps->colors);
        free(ps);
        return NULL;
    }
    
    return ps;
}

// Initialize grid for spatial partitioning
Grid* CreateGrid(int width, int height, float cell_size) {
    Grid* grid = malloc(sizeof(Grid));
    if (!grid) return NULL;
    
    grid->width = ceil(width / cell_size);
    grid->height = ceil(height / cell_size);
    grid->cell_size = cell_size;
    grid->cells = malloc(sizeof(GridCell) * grid->width * grid->height);
    
    if (!grid->cells) {
        free(grid);
        return NULL;
    }
    
    // Initialize grid cells
    for (int i = 0; i < grid->width * grid->height; i++) {
        grid->cells[i].particles = malloc(sizeof(int) * 32); // Initial capacity
        grid->cells[i].capacity = 32;
        grid->cells[i].count = 0;
        
        if (!grid->cells[i].particles) {
            // Handle allocation failure
            for (int j = 0; j < i; j++) {
                free(grid->cells[j].particles);
            }
            free(grid->cells);
            free(grid);
            return NULL;
        }
    }
    
    return grid;
}

// Update grid with current particle positions
void UpdateGrid(Grid* grid, ParticleSystem* ps) {
    // Clear grid
    for (int i = 0; i < grid->width * grid->height; i++) {
        grid->cells[i].count = 0;
    }
    
    // Add particles to grid
    for (int i = 0; i < ps->count; i++) {
        int cell_x = (int)(ps->pos_x[i] / grid->cell_size);
        int cell_y = (int)(ps->pos_y[i] / grid->cell_size);
        
        // Clamp to grid bounds
        if (cell_x < 0) cell_x = 0;
        if (cell_x >= grid->width) cell_x = grid->width - 1;
        if (cell_y < 0) cell_y = 0;
        if (cell_y >= grid->height) cell_y = grid->height - 1;
        
        int cell_idx = cell_y * grid->width + cell_x;
        
        // Resize if needed
        if (grid->cells[cell_idx].count >= grid->cells[cell_idx].capacity) {
            grid->cells[cell_idx].capacity *= 2;
            grid->cells[cell_idx].particles = realloc(
                grid->cells[cell_idx].particles, 
                sizeof(int) * grid->cells[cell_idx].capacity
            );
        }
        
        grid->cells[cell_idx].particles[grid->cells[cell_idx].count++] = i;
    }
}

// Fast distance squared (avoid sqrt for distance comparisons)
inline float DistanceSquared(float x1, float y1, float x2, float y2) {
    float dx = x1 - x2;
    float dy = y1 - y2;
    return dx * dx + dy * dy;
}

// Calculate particle interactions using grid
void CalculateForces(ParticleSystem* ps, Grid* grid, float** interaction_matrix, 
                     float r_min, float r_max, float repulse) {
    float r_min_sq = r_min * r_min;
    float r_max_sq = r_max * r_max;
    float r_mid_sq = ((r_min + r_max) / 2.0f) * ((r_min + r_max) / 2.0f);
    
    // Reset forces
    #pragma omp parallel for
    for (int i = 0; i < ps->count; i++) {
        ps->force_x[i] = 0.0f;
        ps->force_y[i] = 0.0f;
    }
    
    // Calculate forces using grid
    #pragma omp parallel for schedule(dynamic)
    for (int cell_y = 0; cell_y < grid->height; cell_y++) {
        for (int cell_x = 0; cell_x < grid->width; cell_x++) {
            int cell_idx = cell_y * grid->width + cell_x;
            GridCell* cell = &grid->cells[cell_idx];
            
            // Check interactions within this cell
            for (int i = 0; i < cell->count; i++) {
                int p1 = cell->particles[i];
                
                // Check against other particles in same cell
                for (int j = i + 1; j < cell->count; j++) {
                    int p2 = cell->particles[j];
                    
                    float dx = ps->pos_x[p2] - ps->pos_x[p1];
                    float dy = ps->pos_y[p2] - ps->pos_y[p1];
                    float dist_sq = dx * dx + dy * dy;
                    
                    if (dist_sq > 0.0f && dist_sq < r_max_sq) {
                        float dist = sqrtf(dist_sq);  // Only use sqrt when needed
                        float dir_x = dx / dist;
                        float dir_y = dy / dist;
                        
                        int color1 = ps->colors[p1];
                        int color2 = ps->colors[p2];
                        
                        float c1 = 2.0f * interaction_matrix[color1][color2] / (r_max - r_min);
                        float c2 = 2.0f * interaction_matrix[color2][color1] / (r_max - r_min);
                        
                        // Using atomic operations to avoid race conditions
                        if (dist_sq < r_min_sq) {
                            float force = dist * (-repulse + (repulse / r_min));
                            #pragma omp atomic
                            ps->force_x[p1] += force * dir_x;
                            #pragma omp atomic
                            ps->force_y[p1] += force * dir_y;
                            #pragma omp atomic
                            ps->force_x[p2] -= force * dir_x;
                            #pragma omp atomic
                            ps->force_y[p2] -= force * dir_y;
                        } 
                        else if (dist_sq < r_mid_sq) {
                            #pragma omp atomic
                            ps->force_x[p1] += dist * c1 * dir_x;
                            #pragma omp atomic
                            ps->force_y[p1] += dist * c1 * dir_y;
                            #pragma omp atomic
                            ps->force_x[p2] -= dist * c2 * dir_x;
                            #pragma omp atomic
                            ps->force_y[p2] -= dist * c2 * dir_y;
                        } 
                        else {
                            #pragma omp atomic
                            ps->force_x[p1] -= c1 * dir_x;
                            #pragma omp atomic
                            ps->force_y[p1] -= c1 * dir_y;
                            #pragma omp atomic
                            ps->force_x[p2] += c2 * dir_x;
                            #pragma omp atomic
                            ps->force_y[p2] += c2 * dir_y;
                        }
                    }
                }
                
                // Check against neighboring cells (only in positive direction to avoid duplicates)
                for (int ny = cell_y; ny <= cell_y + 1 && ny < grid->height; ny++) {
                    for (int nx = (ny == cell_y) ? cell_x + 1 : cell_x - 1; 
                         nx <= cell_x + 1 && nx < grid->width; nx++) {
                        
                        if (nx < 0) continue;
                        
                        int ncell_idx = ny * grid->width + nx;
                        GridCell* ncell = &grid->cells[ncell_idx];
                        
                        for (int j = 0; j < ncell->count; j++) {
                            int p2 = ncell->particles[j];
                            
                            float dx = ps->pos_x[p2] - ps->pos_x[p1];
                            float dy = ps->pos_y[p2] - ps->pos_y[p1];
                            float dist_sq = dx * dx + dy * dy;
                            
                            if (dist_sq > 0.0f && dist_sq < r_max_sq) {
                                float dist = sqrtf(dist_sq);
                                float dir_x = dx / dist;
                                float dir_y = dy / dist;
                                
                                int color1 = ps->colors[p1];
                                int color2 = ps->colors[p2];
                                
                                float c1 = 2.0f * interaction_matrix[color1][color2] / (r_max - r_min);
                                float c2 = 2.0f * interaction_matrix[color2][color1] / (r_max - r_min);
                                
                                if (dist_sq < r_min_sq) {
                                    float force = dist * (-repulse + (repulse / r_min));
                                    #pragma omp atomic
                                    ps->force_x[p1] += force * dir_x;
                                    #pragma omp atomic
                                    ps->force_y[p1] += force * dir_y;
                                    #pragma omp atomic
                                    ps->force_x[p2] -= force * dir_x;
                                    #pragma omp atomic
                                    ps->force_y[p2] -= force * dir_y;
                                } 
                                else if (dist_sq < r_mid_sq) {
                                    #pragma omp atomic
                                    ps->force_x[p1] += dist * c1 * dir_x;
                                    #pragma omp atomic
                                    ps->force_y[p1] += dist * c1 * dir_y;
                                    #pragma omp atomic
                                    ps->force_x[p2] -= dist * c2 * dir_x;
                                    #pragma omp atomic
                                    ps->force_y[p2] -= dist * c2 * dir_y;
                                } 
                                else {
                                    #pragma omp atomic
                                    ps->force_x[p1] -= c1 * dir_x;
                                    #pragma omp atomic
                                    ps->force_y[p1] -= c1 * dir_y;
                                    #pragma omp atomic
                                    ps->force_x[p2] += c2 * dir_x;
                                    #pragma omp atomic
                                    ps->force_y[p2] += c2 * dir_y;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// Update particle positions
void UpdateParticles(ParticleSystem* ps, float time_step, float damping, int width, int height) {
    #pragma omp parallel for
    for (int i = 0; i < ps->count; i++) {
        // Update velocity
        ps->vel_x[i] += ps->force_x[i] / time_step;
        ps->vel_y[i] += ps->force_y[i] / time_step;
        
        // Apply damping
        ps->vel_x[i] *= damping;
        ps->vel_y[i] *= damping;
        
        // Update position
        ps->pos_x[i] += ps->vel_x[i] / time_step;
        ps->pos_y[i] += ps->vel_y[i] / time_step;
        
        // Boundary handling - wrap around
        if (ps->pos_x[i] < 0) ps->pos_x[i] += width;
        if (ps->pos_x[i] >= width) ps->pos_x[i] -= width;
        if (ps->pos_y[i] < 0) ps->pos_y[i] += height;
        if (ps->pos_y[i] >= height) ps->pos_y[i] -= height;
    }
}

void SDL_ExitWithError(const char *message) {
    SDL_Log("Error: %s > %s\n", message, SDL_GetError());
    exit(EXIT_FAILURE);
}

int main() {
    srand(time(0));
    
    // Simulation parameters
    int width = 800;
    int height = 800;
    float r_min = 5.0f;
    float r_max = 300.0f;
    int num_colors = 3;
    int num_particles = 1000;
    float time_step = 100.0f;
    float damping = 0.993f;
    
    // Set number of threads for OpenMP
    omp_set_num_threads(omp_get_num_procs());
    
    // Create interaction matrix
    float** interaction_matrix = malloc(sizeof(float*) * num_colors);
    for (int i = 0; i < num_colors; i++) {
        interaction_matrix[i] = malloc(sizeof(float) * num_colors);
        for (int j = 0; j < num_colors; j++) {
            interaction_matrix[i][j] = -2.0f + 4.0f * (float)(rand() % 100) / 100.0f;
        }
    }
    
    // Create particle system
    ParticleSystem* ps = CreateParticleSystem(num_particles);
    
    // Initialize particles
    for (int i = 0; i < num_particles; i++) {
        ps->pos_x[i] = (float)(rand() % width);
        ps->pos_y[i] = (float)(rand() % height);
        ps->vel_x[i] = 0.0f;
        ps->vel_y[i] = 0.0f;
        ps->force_x[i] = 0.0f;
        ps->force_y[i] = 0.0f;
        ps->colors[i] = rand() % num_colors;
    }
    
    // Create spatial grid (cell size should be >= r_max for optimal efficiency)
    Grid* grid = CreateGrid(width, height, r_max);
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO))
        SDL_ExitWithError("Failed to initialize SDL");
    
    SDL_Window *window = SDL_CreateWindow("Particle Life Optimized", 
                                          SDL_WINDOWPOS_CENTERED, 0, 
                                          width, height, 0);
    if (!window)
        SDL_ExitWithError("Window creation failed");
    
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
        SDL_ExitWithError("Renderer creation failed");
    
    // Enable VSync for smoother rendering
    SDL_RenderSetVSync(renderer, 1);
    
    // Main loop
    SDL_bool running = SDL_TRUE;
    Uint32 frame_start, frame_time;
    float fps = 0.0f;
    
    while (running) {
        frame_start = SDL_GetTicks();
        
        // Handle events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = SDL_FALSE;
            }
        }
        
        // Clear screen
        SDL_SetRenderDrawColor(renderer, 30, 10, 10, 255);
        SDL_RenderClear(renderer);
        
        // Update grid
        UpdateGrid(grid, ps);
        
        // Calculate forces
        CalculateForces(ps, grid, interaction_matrix, r_min, r_max, 10.0f);
        
        // Update particles
        UpdateParticles(ps, time_step, damping, width, height);
        
        // Draw particles
        for (int i = 0; i < ps->count; i++) {
            int color = ps->colors[i];
            SDL_SetRenderDrawColor(renderer, 
                                  255 * (color == 1), 
                                  255 * (color == 0), 
                                  255 * (color == 2), 
                                  255);
            SDL_RenderDrawPoint(renderer, (int)ps->pos_x[i], (int)ps->pos_y[i]);
        }
        
        // Present render
        SDL_RenderPresent(renderer);
        
        // Calculate FPS
        frame_time = SDL_GetTicks() - frame_start;
        if (frame_time > 0) {
            fps = 1000.0f / frame_time;
        }
        
        // Display FPS in window title
        char title[64];
        sprintf(title, "Particle Life Optimized - FPS: %.1f", fps);
        SDL_SetWindowTitle(window, title);
    }
    
    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    // Free memory
    free(ps->pos_x);
    free(ps->pos_y);
    free(ps->vel_x);
    free(ps->vel_y);
    free(ps->force_x);
    free(ps->force_y);
    free(ps->colors);
    free(ps);
    
    for (int i = 0; i < grid->width * grid->height; i++) {
        free(grid->cells[i].particles);
    }
    free(grid->cells);
    free(grid);
    
    for (int i = 0; i < num_colors; i++) {
        free(interaction_matrix[i]);
    }
    free(interaction_matrix);
    
    return EXIT_SUCCESS;
}
