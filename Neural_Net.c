#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>

// Neural Network structures
typedef double (*ActivationFunction)(double);

typedef struct Position {
    int x;
    int y;
} Position;

typedef struct Neuron {
    double value;
    double bias;
    ActivationFunction activation_func;
    struct Connection* inputs;
    int num_inputs;
    struct Connection* outputs;
    int num_outputs;
    Position pos;  // Added for visualization
    int layer;     // Added for visualization
} Neuron;

typedef struct Connection {
    struct Neuron* from;
    struct Neuron* to;
    double weight;
} Connection;

typedef struct InputControl {
    SDL_Rect rect;        // Rectangle for drawing
    double* target_value; // Pointer to neuron value to control
    double current_value; // Current input value (0.0 to 1.0)
} InputControl;

const int CONTROL_SIZE = 30;
const int CONTROL_SPACING = 10;
const int CONTROL_SEGMENTS = 10; 
InputControl create_input_control(int x, int y, double* target) {
    InputControl ctrl;
    ctrl.rect.x = x;
    ctrl.rect.y = y;
    ctrl.rect.w = CONTROL_SIZE;
    ctrl.rect.h = CONTROL_SIZE;
    ctrl.target_value = target;
    ctrl.current_value = 0.0;
    return ctrl;
}

// Function to check if a point is inside a rectangle
SDL_bool point_in_rect(int x, int y, SDL_Rect* rect) {
    return (x >= rect->x && x <= rect->x + rect->w &&
            y >= rect->y && y <= rect->y + rect->h);
}

// Function to draw input control
void draw_input_control(SDL_Renderer* renderer, InputControl* ctrl) {
    // Draw outline
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &ctrl->rect);
    
    // Draw filled portion based on value
    SDL_Rect fill_rect = ctrl->rect;
    fill_rect.h = (int)(ctrl->rect.h * ctrl->current_value);
    fill_rect.y = ctrl->rect.y + ctrl->rect.h - fill_rect.h;
    
    // Use a color gradient from blue (low) to red (high)
    Uint8 r = (Uint8)(255 * ctrl->current_value);
    Uint8 b = (Uint8)(255 * (1.0 - ctrl->current_value));
    SDL_SetRenderDrawColor(renderer, r, 0, b, 255);
    SDL_RenderFillRect(renderer, &fill_rect);
}

Neuron* create_neuron(double bias, ActivationFunction func) {
    Neuron* neuron = (Neuron*)malloc(sizeof(Neuron));
    neuron->value = 0.0;
    neuron->bias = bias;
    neuron->activation_func = func;
    neuron->inputs = NULL;
    neuron->num_inputs = 0;
    neuron->outputs = NULL;
    neuron->num_outputs = 0;
    return neuron;
}


// Create a connection between two neurons
Connection* create_connection(Neuron* from, Neuron* to, double weight) {
    Connection* conn = (Connection*)malloc(sizeof(Connection));
    conn->from = from;
    conn->to = to;
    conn->weight = weight;
    
    // Add to source neuron's outputs
    from->outputs = (Connection*)realloc(from->outputs, 
                                       (from->num_outputs + 1) * sizeof(Connection));
    from->outputs[from->num_outputs++] = *conn;
    
    // Add to target neuron's inputs
    to->inputs = (Connection*)realloc(to->inputs, 
                                    (to->num_inputs + 1) * sizeof(Connection));
    to->inputs[to->num_inputs++] = *conn;
    
    return conn;
}

// Visualization constants
const int NEURON_RADIUS = 20;
const int LAYER_SPACING = 150;
const int VERTICAL_SPACING = 80;
const int WINDOW_HEIGHT = 600;
const int WINDOW_WIDTH = 800;

// Color utilities for visualization
void get_activation_color(double value, Uint8* r, Uint8* g, Uint8* b) {
    // Convert activation value to color (blue to red gradient)
    *r = (Uint8)(255 * value);
    *g = 0;
    *b = (Uint8)(255 * (1.0 - value));
}

void get_weight_color(double weight, Uint8* r, Uint8* g, Uint8* b) {
    // Negative weights are red, positive are green
    if (weight < 0) {
        *r = (Uint8)(255 * fmin(-weight, 1.0));
        *g = 0;
        *b = 0;
    } else {
        *r = 0;
        *g = (Uint8)(255 * fmin(weight, 1.0));
        *b = 0;
    }
}
double sigmoid(double x) {
    return 1.0 / (1.0 + exp(-x));
}

double relu(double x) {
    return x > 0 ? x : 0;
}

double tanh_activation(double x) {
    return tanh(x);
}
void forward_propagate_neuron(Neuron* neuron) {
    double sum = neuron->bias;  // Start with the bias
    
    // Sum up all inputs * weights
    for (int i = 0; i < neuron->num_inputs; i++) {
        Connection input = neuron->inputs[i];
        sum += input.from->value * input.weight;
    }
    
    // Apply activation function
    neuron->value = neuron->activation_func(sum);
}

// Drawing functions
void draw_neuron(SDL_Renderer* renderer, Neuron* neuron) {
    
    // Draw neuron circle
    Uint8 r, g, b;
    get_activation_color(sigmoid(neuron->value), &r, &g, &b);
    
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    
    // Draw filled circle
    for (int w = 0; w < NEURON_RADIUS * 2; w++) {
        for (int h = 0; h < NEURON_RADIUS * 2; h++) {
            int dx = NEURON_RADIUS - w;
            int dy = NEURON_RADIUS - h;
            if ((dx*dx + dy*dy) <= (NEURON_RADIUS * NEURON_RADIUS)) {
                SDL_RenderDrawPoint(renderer, 
                                  neuron->pos.x + dx, 
                                  neuron->pos.y + dy);
            }
        }
    }
    
    // Draw outline
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    int resolution = 30;
    for (int i = 0; i < resolution; i++) {
        double angle1 = 2.0 * M_PI * i / resolution;
        double angle2 = 2.0 * M_PI * (i + 1) / resolution;
        
        SDL_RenderDrawLine(renderer,
                          neuron->pos.x + NEURON_RADIUS * cos(angle1),
                          neuron->pos.y + NEURON_RADIUS * sin(angle1),
                          neuron->pos.x + NEURON_RADIUS * cos(angle2),
                          neuron->pos.y + NEURON_RADIUS * sin(angle2));
    }
}

void draw_connection(SDL_Renderer* renderer, Connection* conn) {
    // Draw line for connection
    Uint8 r, g, b;
    get_weight_color(conn->weight, &r, &g, &b);
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    
    SDL_RenderDrawLine(renderer,
                      conn->from->pos.x + NEURON_RADIUS,
                      conn->from->pos.y,
                      conn->to->pos.x - NEURON_RADIUS,
                      conn->to->pos.y);
}

void draw_network(SDL_Renderer* renderer, Neuron** neurons, int num_neurons) {
    // Clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    // Draw connections first (so they appear behind neurons)
    for (int i = 0; i < num_neurons; i++) {
        for (int j = 0; j < neurons[i]->num_outputs; j++) {
            draw_connection(renderer, &neurons[i]->outputs[j]);
        }
    }
    
    // Draw neurons
    for (int i = 0; i < num_neurons; i++) {
        draw_neuron(renderer, neurons[i]);
    }
    
    SDL_RenderPresent(renderer);
}

// Layout function to position neurons
void layout_network(Neuron** neurons, int num_neurons, int num_layers) {
    int* neurons_per_layer = calloc(num_layers, sizeof(int));
    
    // Count neurons per layer
    for (int i = 0; i < num_neurons; i++) {
        neurons_per_layer[neurons[i]->layer]++;
    }
    
    // Position neurons
    int* layer_counts = calloc(num_layers, sizeof(int));
    for (int i = 0; i < num_neurons; i++) {
        int layer = neurons[i]->layer;
        int layer_size = neurons_per_layer[layer];
        int position = layer_counts[layer]++;
        
        neurons[i]->pos.x = 100 + layer * LAYER_SPACING;
        neurons[i]->pos.y = WINDOW_HEIGHT/2 - (layer_size-1)*VERTICAL_SPACING/2 
                           + position * VERTICAL_SPACING;
    }
    
    free(neurons_per_layer);
    free(layer_counts);
}

// Main visualization loop
void visualize_network(Neuron** neurons, int num_neurons, int num_layers) {
    SDL_Window* window;
    SDL_Renderer* renderer;
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        return;
    }
    
    window = SDL_CreateWindow("Neural Network Visualization",
                            SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED,
                            WINDOW_WIDTH, WINDOW_HEIGHT,
                            SDL_WINDOW_SHOWN);
    
    if (!window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        return;
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        return;
    }
    InputControl* controls = NULL;
    int num_controls = 0;
    for (int i = 0; i < num_neurons; i++) {
        if (neurons[i]->layer == 0) { // Input layer
            num_controls++;
            controls = realloc(controls, num_controls * sizeof(InputControl));
            controls[num_controls - 1] = create_input_control(
                50,  // X position (left of the neuron)
                neurons[i]->pos.y - CONTROL_SIZE/2,  // Y position (centered with neuron)
                &neurons[i]->value  // Point to neuron's value
            );
        }
    }
 
    
    SDL_bool running = SDL_TRUE;
 while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = SDL_FALSE;
                    break;
                    
                case SDL_MOUSEBUTTONDOWN:
                    mouse_down = SDL_TRUE;
                    // Fallthrough intentional
                    
                case SDL_MOUSEMOTION:
                    if (mouse_down) {
                        int mouse_x = event.motion.x;
                        int mouse_y = event.motion.y;
                        
                        // Check each input control
                        for (int i = 0; i < num_controls; i++) {
                            if (point_in_rect(mouse_x, mouse_y, &controls[i].rect)) {
                                // Calculate value based on vertical position
                                double value = 1.0 - ((double)(mouse_y - controls[i].rect.y) / controls[i].rect.h);
                                value = fmax(0.0, fmin(1.0, value)); // Clamp between 0 and 1
                                controls[i].current_value = value;
                                *controls[i].target_value = value;
                                
                                // Propagate through network
                                for (int j = 1; j < num_layers; j++) {
                                    for (int k = 0; k < num_neurons; k++) {
                                        if (neurons[k]->layer == j) {
                                            forward_propagate_neuron(neurons[k]);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    break;
                    
                case SDL_MOUSEBUTTONUP:
                    mouse_down = SDL_FALSE;
                    break;
            }
        }
        
        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        // Draw network
        draw_network(renderer, neurons, num_neurons);
        
        // Draw input controls
        for (int i = 0; i < num_controls; i++) {
            draw_input_control(renderer, &controls[i]);
        }
        
        // Draw output values
        for (int i = 0; i < num_neurons; i++) {
            if (neurons[i]->layer == num_layers - 1) { // Output layer
                char value_text[32];
                snprintf(value_text, sizeof(value_text), "%.3f", neurons[i]->value);
                // Note: In a full implementation, you'd want to use SDL_ttf for text rendering
                // For now, we'll rely on the neuron color to indicate the output value
            }
        }
        
        SDL_RenderPresent(renderer);
        SDL_Delay(16);  // Cap at ~60 FPS
    }
    
    // Cleanup
    free(controls);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
void free_neuron(Neuron* neuron) {
    if (neuron) {
        free(neuron->inputs);
        free(neuron->outputs);
        free(neuron);
    }
}

// Example usage
int main(int argc, char** argv) {
    // Create a simple network
    Neuron* input1 = create_neuron(0.0, NULL);
    Neuron* input2 = create_neuron(0.0, NULL);
    Neuron* hidden1 = create_neuron(0.5, sigmoid);
    Neuron* hidden2 = create_neuron(0.5, sigmoid);
    Neuron* output = create_neuron(-0.1, sigmoid);
    
    // Set layer information for visualization
    input1->layer = 0;
    input2->layer = 0;
    hidden1->layer = 1;
    hidden2->layer = 1;
    output->layer = 2;
    
    // Create connections
    create_connection(input1, hidden1, 0.7);
    create_connection(input1, hidden2, -0.3);
    create_connection(input2, hidden1, 0.5);
    create_connection(input2, hidden2, 0.8);
    create_connection(hidden1, output, 0.4);
    create_connection(hidden2, output, 0.9);
    
    // Set some example values
    input1->value = 0.8;
    input2->value = 0.4;
    
    // Forward propagate
    forward_propagate_neuron(hidden1);
    forward_propagate_neuron(hidden2);
    forward_propagate_neuron(output);
    
    // Create array of neurons for visualization
    Neuron* neurons[] = {input1, input2, hidden1, hidden2, output};
    
    // Visualize the network
    visualize_network(neurons, 5, 3);
    
    // Clean up
    free_neuron(input1);
    free_neuron(input2);
    free_neuron(hidden1);
    free_neuron(hidden2);
    free_neuron(output);
    
    return 0;
}