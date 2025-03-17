#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <SDL2/SDL.h>

#define WIDTH 800
#define HEIGHT 800
#define GRID_SIZE 100
#define DX (WIDTH / GRID_SIZE)
#define DT 0.01
#define G 0.1

// Grid arrays
float density[GRID_SIZE][GRID_SIZE] = {0};
float potential[GRID_SIZE][GRID_SIZE] = {0};
float velocity_x[GRID_SIZE][GRID_SIZE] = {0};
float velocity_y[GRID_SIZE][GRID_SIZE] = {0};

// Initialize density with a central concentration and rotation
void initialize_density() {
    // Set random seed based on time
    srand(time(NULL));
    
    float max_density = 1.0;
    float max_velocity = 0.5;
    
    // Initialize with pure random values
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            // Random density between 0 and 1
            density[i][j] = (float)rand() / RAND_MAX;
            
            // Random velocities between -max_velocity and +max_velocity
            velocity_x[i][j] = ((float)rand() / RAND_MAX * 2.0 - 1.0) * max_velocity;
            velocity_y[i][j] = ((float)rand() / RAND_MAX * 2.0 - 1.0) * max_velocity;
        }
    }
}

// Solve Poisson equation for gravitational potential using Gauss-Seidel relaxation
void solve_poisson() {
    float residual = 0.0;
    float epsilon = 1e-6;  // Convergence threshold
    int max_iter = 200;    // Maximum iterations
    
    for (int iter = 0; iter < max_iter; iter++) {
        residual = 0.0;
        for (int i = 1; i < GRID_SIZE - 1; i++) {
            for (int j = 1; j < GRID_SIZE - 1; j++) {
                float old_potential = potential[i][j];
                potential[i][j] = 0.25 * (potential[i + 1][j] + potential[i - 1][j] +
                                        potential[i][j + 1] + potential[i][j - 1] -
                                        DX * DX * (4 * M_PI * G * density[i][j]));
                residual += fabs(potential[i][j] - old_potential);
            }
        }
        if (residual < epsilon) break;
    }
}

// Compute acceleration from gravitational potential
void compute_gravity() {
    const float max_acceleration = 5.0; // Limit maximum acceleration
    
    for (int i = 1; i < GRID_SIZE - 1; i++) {
        for (int j = 1; j < GRID_SIZE - 1; j++) {
            // Compute accelerations
            float ax = -(potential[i + 1][j] - potential[i - 1][j]) / (2 * DX);
            float ay = -(potential[i][j + 1] - potential[i][j - 1]) / (2 * DX);
            
            // Limit acceleration magnitude
            float a_mag = sqrt(ax*ax + ay*ay);
            if (a_mag > max_acceleration) {
                float scale = max_acceleration / a_mag;
                ax *= scale;
                ay *= scale;
            }
            
            // Update velocities
            velocity_x[i][j] += ax * DT;
            velocity_y[i][j] += ay * DT;
            
            // Apply velocity damping to prevent numerical instability
            velocity_x[i][j] *= 0.999;
            velocity_y[i][j] *= 0.999;
        }
    }
}

// Advect density using semi-Lagrangian scheme with bilinear interpolation and mass conservation
void advect_density() {
    float new_density[GRID_SIZE][GRID_SIZE] = {0};
    float total_mass_before = 0;
    float total_mass_after = 0;
    
    // Calculate total mass before advection
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            total_mass_before += density[i][j];
        }
    }
    
    // Velocity limiter to prevent too large movements
    const float max_velocity = 2.0;
    
    for (int i = 1; i < GRID_SIZE - 1; i++) {
        for (int j = 1; j < GRID_SIZE - 1; j++) {
            // Limit velocity magnitude
            float vx = velocity_x[i][j];
            float vy = velocity_y[i][j];
            float v_mag = sqrt(vx*vx + vy*vy);
            if (v_mag > max_velocity) {
                float scale = max_velocity / v_mag;
                vx *= scale;
                vy *= scale;
            }
            
            // Backtrace position
            float x = i - vx * DT / DX;
            float y = j - vy * DT / DX;
            
            // Ensure we stay within bounds
            x = fmax(0.5f, fmin(GRID_SIZE - 1.5f, x));
            y = fmax(0.5f, fmin(GRID_SIZE - 1.5f, y));
            
            // Bilinear interpolation
            int x0 = (int)x;
            int y0 = (int)y;
            float fx = x - x0;
            float fy = y - y0;
            
            new_density[i][j] = (1-fx)*(1-fy)*density[x0][y0] +
                               fx*(1-fy)*density[x0+1][y0] +
                               (1-fx)*fy*density[x0][y0+1] +
                               fx*fy*density[x0+1][y0+1];
        }
    }
    
    // Calculate total mass after advection
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            total_mass_after += new_density[i][j];
        }
    }
    
    // Apply correction factor to conserve mass
    if (total_mass_after > 0) {
        float correction = total_mass_before / total_mass_after;
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                new_density[i][j] *= correction;
            }
        }
    }
    
    memcpy(density, new_density, sizeof(density));
}

// Add density diffusion to smooth out numerical instabilities
void diffuse_density() {
    float diff_rate = 0.1;
    float new_density[GRID_SIZE][GRID_SIZE] = {0};
    for (int i = 1; i < GRID_SIZE - 1; i++) {
        for (int j = 1; j < GRID_SIZE - 1; j++) {
            new_density[i][j] = density[i][j] + diff_rate * (
                density[i+1][j] + density[i-1][j] +
                density[i][j+1] + density[i][j-1] - 
                4 * density[i][j]
            );
        }
    }
    memcpy(density, new_density, sizeof(density));
}

// Apply boundary conditions to prevent mass loss
void apply_boundary_conditions() {
    // Reflective boundaries
    for (int i = 0; i < GRID_SIZE; i++) {
        density[0][i] = density[1][i];
        density[GRID_SIZE-1][i] = density[GRID_SIZE-2][i];
        density[i][0] = density[i][1];
        density[i][GRID_SIZE-1] = density[i][GRID_SIZE-2];
        
        velocity_x[0][i] = -velocity_x[1][i];
        velocity_x[GRID_SIZE-1][i] = -velocity_x[GRID_SIZE-2][i];
        velocity_y[i][0] = -velocity_y[i][1];
        velocity_y[i][GRID_SIZE-1] = -velocity_y[i][GRID_SIZE-2];
    }
}

// Enhanced render function with velocity-based coloring
void render(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            // Calculate velocity magnitude for coloring
            float vel_mag = sqrt(velocity_x[i][j]*velocity_x[i][j] + 
                               velocity_y[i][j]*velocity_y[i][j]);
            
            // Normalize velocity magnitude for coloring
            float vel_color = vel_mag * 5.0; // Scale factor for visibility
            if (vel_color > 1.0) vel_color = 1.0;
            
            // Create color based on density (blue) and velocity (red)
            int blue = 255 - (int)(density[i][j] * 255.0);
            int red = (int)(vel_color * 255.0);
            if (blue < 0) blue = 0;
            if (red < 0) red = 0;
            
            SDL_SetRenderDrawColor(renderer, red, 0, blue, 255);
            SDL_Rect rect = {i * DX, j * DX, DX, DX};
            SDL_RenderFillRect(renderer, &rect);
        }
    }
    SDL_RenderPresent(renderer);
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Improved Grid-Based Gravity", 
                                        SDL_WINDOWPOS_CENTERED, 
                                        SDL_WINDOWPOS_CENTERED, 
                                        WIDTH, HEIGHT, 
                                        SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    initialize_density();
    int running = 1;
    SDL_Event event;
    
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
            
            // Add mouse interaction
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                int x = event.button.x / DX;
                int y = event.button.y / DX;
                if (x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE) {
                    // Add matter where clicked
                    density[x][y] += 1.0;
                }
            }
        }
        
        solve_poisson();
        compute_gravity();
        advect_density();
        diffuse_density();
        apply_boundary_conditions();
        render(renderer);
        
        SDL_Delay(16); // Cap at ~60 FPS
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}