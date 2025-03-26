#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <omp.h>
#include <math.h>
#include <stdbool.h>
#include "Mathutils.h"

quat C={-0.4, 0.6, 0, 0};

// Structure for 3D vector
typedef struct {
    float x, y, z;
} Vector3;

// Structure for ray
typedef struct {
    Vector3 origin;
    Vector3 direction;
} Ray;

// Structure for camera
typedef struct {
    Vector3 position;
    Vector3 target;
    float yaw;        // Left/right rotation (in radians)
    float pitch;      // Up/down rotation (in radians)
    float roll;       // Roll rotation (in radians)
    float fov;        // Field of view in degrees
    float speed;      // Movement speed
} Camera;

// Structure for color
typedef struct {
    Uint8 r, g, b, a;
} Color;

void SDL_Exitwitherror(const char *message)
{
    SDL_Log("Erreur:%s >%s \n", message, SDL_GetError());
    exit(EXIT_FAILURE);
}

// Modified TestDiverg to return the actual iteration count and normalized value
typedef struct {
    float value;       // Normalized value (0-1)
    int iterations;    // Actual iteration count
    bool escaped;      // Whether it escaped or not
} DivergenceResult;

DivergenceResult TestDivergEnhanced(int maxIters, quat (*f)(quat), quat a, double escapeRadius)
{
    DivergenceResult result;
    quat iteration = a;
    int i;
    
    for(i = 0; i < maxIters; i++)
    {
        iteration = f(iteration);
        double mag = module_q(iteration);
        
        if(mag > escapeRadius) {
            // It escaped - calculate smooth coloring value
            // log(log(mag)) / log(2) is a common smoothing formula for fractals
            float smooth = i + 1 - log2f(logf(mag) / logf(escapeRadius));
            result.value = smooth / maxIters;
            result.iterations = i;
            result.escaped = true;
            return result;
        }
    }
    
    // It did not escape within maxIters
    result.value = 0.0f;
    result.iterations = maxIters;
    result.escaped = false;
    return result;
}

quat Mendel(quat in)
{
    return sum_q(Pow_q(in, 2), C);
}

// Vector operations
Vector3 vectorAdd(Vector3 a, Vector3 b) {
    Vector3 result = {a.x + b.x, a.y + b.y, a.z + b.z};
    return result;
}

Vector3 vectorSubtract(Vector3 a, Vector3 b) {
    Vector3 result = {a.x - b.x, a.y - b.y, a.z - b.z};
    return result;
}

Vector3 vectorScale(Vector3 v, float scale) {
    Vector3 result = {v.x * scale, v.y * scale, v.z * scale};
    return result;
}

Vector3 vectorCross(Vector3 a, Vector3 b) {
    Vector3 result = {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
    return result;
}

float vectorDot(Vector3 a, Vector3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float vectorLength(Vector3 v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vector3 vectorNormalize(Vector3 v) {
    float length = vectorLength(v);
    if (length > 0.0001f) {
        v.x /= length;
        v.y /= length;
        v.z /= length;
    }
    return v;
}

// Ray casting through the volume to find Julia Set
// Returns information about the ray hit
typedef struct {
    bool hit;                  // Whether the ray hit anything
    float distance;            // Distance to the hit
    DivergenceResult result;   // Divergence result at the hit point
    float opacity;             // Opacity of the hit
} RayHit;

RayHit rayMarchJulia(Ray ray, int maxSteps, float stepSize, int maxIters, float escapeRadius)
{
    RayHit hit;
    hit.hit = false;
    hit.distance = 0.0f;
    hit.opacity = 0.0f;
    
    Vector3 currentPos;
    
    for (int i = 0; i < maxSteps; i++) {
        // Advance the ray
        hit.distance += stepSize;
        currentPos = vectorAdd(ray.origin, vectorScale(ray.direction, hit.distance));
        
        // Convert position to quaternion (4th component is fixed to 0 to get a 3D slice)
        quat position = {currentPos.x, currentPos.y, currentPos.z, 0.0};
        
        // Test for divergence
        hit.result = TestDivergEnhanced(maxIters, Mendel, position, escapeRadius);
        
        // If we're close to the Julia set boundary
        if (!hit.result.escaped) {
            hit.hit = true;
            hit.opacity = 1.0f;
            return hit;
        }
        else if (hit.result.value > 0.01f && hit.result.value < 0.99f) {
            // We're in the transition region - accumulate some opacity
            hit.hit = true;
            hit.opacity = 1.0f - hit.result.value;
            if (hit.opacity > 0.1f) {
                return hit;
            }
        }
        
        // Break if we've gone too far
        if (hit.distance > 10.0f) {
            break;
        }
    }
    
    return hit;
}

// Get forward, right and up vectors from camera orientation
void getCameraVectors(Camera camera, Vector3 *forward, Vector3 *right, Vector3 *up) {
    // Forward is based on yaw and pitch (spherical coordinates)
    forward->x = cosf(camera.yaw) * cosf(camera.pitch);
    forward->y = sinf(camera.pitch);
    forward->z = sinf(camera.yaw) * cosf(camera.pitch);
    
    // Right is perpendicular to forward and world up
    Vector3 worldUp = {0.0f, 1.0f, 0.0f};
    *forward = vectorNormalize(*forward);
    *right = vectorNormalize(vectorCross(*forward, worldUp));
    
    // Up is perpendicular to forward and right
    *up = vectorNormalize(vectorCross(*right, *forward));
    
    // Apply roll if needed
    if (camera.roll != 0.0f) {
        float cosRoll = cosf(camera.roll);
        float sinRoll = sinf(camera.roll);
        Vector3 oldRight = *right;
        Vector3 oldUp = *up;
        
        right->x = oldRight.x * cosRoll - oldUp.x * sinRoll;
        right->y = oldRight.y * cosRoll - oldUp.y * sinRoll;
        right->z = oldRight.z * cosRoll - oldUp.z * sinRoll;
        
        up->x = oldRight.x * sinRoll + oldUp.x * cosRoll;
        up->y = oldRight.y * sinRoll + oldUp.y * cosRoll;
        up->z = oldRight.z * sinRoll + oldUp.z * cosRoll;
    }
}

// Update camera target based on position and orientation
void updateCameraTarget(Camera *camera) {
    Vector3 forward, right, up;
    getCameraVectors(*camera, &forward, &right, &up);
    
    camera->target = vectorAdd(camera->position, forward);
}

// Generate rays from camera for each pixel
Ray generateRay(Camera camera, int x, int y, int width, int height) {
    Ray ray;
    ray.origin = camera.position;
    
    // Calculate camera basis vectors
    Vector3 forward, right, up;
    getCameraVectors(camera, &forward, &right, &up);
    
    // Convert pixel coordinates to NDC space (-1 to 1)
    float ndc_x = (2.0f * (float)x / (float)width) - 1.0f;
    float ndc_y = 1.0f - (2.0f * (float)y / (float)height);
    
    // Adjust for aspect ratio
    float aspect = (float)width / (float)height;
    ndc_x *= aspect;
    
    // Calculate direction based on field of view
    float tan_fov = tanf(camera.fov * 0.5f * M_PI / 180.0f);
    
    // Combine to get ray direction
    Vector3 dir = vectorAdd(
                    vectorScale(forward, 1.0f),
                    vectorAdd(
                        vectorScale(right, ndc_x * tan_fov),
                        vectorScale(up, ndc_y * tan_fov)
                    )
                );
    
    ray.direction = vectorNormalize(dir);
    
    return ray;
}

// Enhanced color mapping based on divergence speed
Color getColorEnhanced(DivergenceResult result, int colorMode, float colorShift) {
    Color color = {0, 0, 0, 255};
    
    // Just check if the point hasn't escaped - no need to check the iterations
    if (!result.escaped) {
        return color; // Background color (black)
    }
    
    // Check if the point is inside the Julia set
    if (!result.escaped) {
        // Inside points are black with high opacity
        color.a = 255;
        return color;
    }
    
    float hue, saturation, value;
    
    switch (colorMode) {
        case 0: // Grayscale based on escape speed
            color.r = color.g = color.b = (Uint8)(255.0f * (1.0f - result.value));
            break;
            
        case 1: // Rainbow based on escape speed
            hue = fmodf(result.value * 360.0f + colorShift, 360.0f);
            saturation = 1.0f;
            value = 1.0f;
            
            // Convert HSV to RGB
            int hi = (int)(hue / 60.0f) % 6;
            float f = hue / 60.0f - hi;
            float p = value * (1.0f - saturation);
            float q = value * (1.0f - f * saturation);
            float tv = value * (1.0f - (1.0f - f) * saturation);
            
            switch (hi) {
                case 0: color.r = value * 255; color.g = tv * 255; color.b = p * 255; break;
                case 1: color.r = q * 255; color.g = value * 255; color.b = p * 255; break;
                case 2: color.r = p * 255; color.g = value * 255; color.b = tv * 255; break;
                case 3: color.r = p * 255; color.g = q * 255; color.b = value * 255; break;
                case 4: color.r = tv * 255; color.g = p * 255; color.b = value * 255; break;
                case 5: color.r = value * 255; color.g = p * 255; color.b = q * 255; break;
            }
            break;
            
        case 2: // "Electric" coloring based on iteration bands
            // Uses a combination of iteration count and smooth value
            float iterFrac = (float)result.iterations / 20.0f; // Creates bands every 20 iterations
            iterFrac = iterFrac - floorf(iterFrac); // Take fractional part
            
            // Combine iteration bands with smooth value for fine detail
            float combinedValue = (iterFrac * 0.8f + result.value * 0.2f);
            
            // Create electric blue-purple-cyan color scheme
            color.r = (Uint8)(128.0f * (sinf(combinedValue * 5.0f + colorShift) * 0.5f + 0.5f));
            color.g = (Uint8)(128.0f * (sinf(combinedValue * 3.0f + colorShift * 0.7f) * 0.5f + 0.5f));
            color.b = (Uint8)(200.0f * (sinf(combinedValue * 2.0f + colorShift * 1.3f) * 0.5f + 0.5f) + 55.0f);
            break;
            
        case 3: // Fire/thermal coloring - goes from black to red to yellow to white
            float tval = 1.0f - result.value; // Invert so small values (fast escape) are bright
            
            if (tval < 0.25f) {
                // Black to red
                float v = tval / 0.25f;
                color.r = v * 255;
                color.g = color.b = 0;
            } 
            else if (tval < 0.5f) {
                // Red to yellow
                float v = (tval - 0.25f) / 0.25f;
                color.r = 255;
                color.g = v * 255;
                color.b = 0;
            }
            else if (tval < 0.75f) {
                // Yellow to white
                float v = (tval - 0.5f) / 0.25f;
                color.r = color.g = 255;
                color.b = v * 255;
            }
            else {
                // White
                color.r = color.g = color.b = 255;
            }
            break;
    }
    
    return color;
}

// Save the rendered image to a BMP file
void saveScreenshot(SDL_Renderer *renderer, int width, int height, const char *filename) {
    SDL_Surface *screenshot = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
    if (!screenshot) {
        printf("Failed to create screenshot surface: %s\n", SDL_GetError());
        return;
    }
    
    if (SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888, screenshot->pixels, screenshot->pitch) != 0) {
        printf("Failed to read pixel data: %s\n", SDL_GetError());
        SDL_FreeSurface(screenshot);
        return;
    }
    
    if (SDL_SaveBMP(screenshot, filename) != 0) {
        printf("Failed to save screenshot: %s\n", SDL_GetError());
    } else {
        printf("Screenshot saved to %s\n", filename);
    }
    
    SDL_FreeSurface(screenshot);
}

int main(int argc, char **argv)
{
    /**********initialization of SDL elements************/
    int size = (argc > 1) ? atoi(argv[1]) : 512;
    SDL_Window *window = NULL;
    SDL_Renderer *rendu = NULL;
    
    // Ray casting parameters
    int maxSteps = 100;              // Maximum ray march steps
    float stepSize = 0.02f;          // Ray march step size
    int maxIters = 15;               // Maximum iterations for Julia set
    float escapeRadius = 4.0f;       // Escape radius for Julia set
    int colorMode = 1;               // Color mode (0=grayscale, 1=rainbow, 2=electric, 3=fire)
    float colorShift = 0.0f;         // Color shifting parameter
    bool autoRotate = false;         // Auto-rotate the view
    
    // Mouse control
    int lastMouseX = 0, lastMouseY = 0;
    bool mouseDown = false;
    bool keyboardMode = true;        // Whether to use keyboard or mouse for camera control
    
    // Camera settings
    Camera camera = {
        .position = {0.0f, 0.0f, -2.5f},
        .target = {0.0f, 0.0f, 0.0f},
        .yaw = 0.0f,
        .pitch = 0.0f,
        .roll = 0.0f,
        .fov = 60.0f,
        .speed = 0.1f
    };
    updateCameraTarget(&camera);
    
    // Result buffer to store calculation results
    RayHit *result_buffer = (RayHit*)malloc(size * size * sizeof(RayHit));
    if (!result_buffer) {
        fprintf(stderr, "Failed to allocate memory for result buffer\n");
        return EXIT_FAILURE;
    }
    
    if(SDL_Init(SDL_INIT_VIDEO))
        SDL_Exitwitherror("failed init");
    
    window = SDL_CreateWindow("Julia Quaternion Ray Casting", SDL_WINDOWPOS_CENTERED, 0, size, size, 0);            
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
    
    // Capture mouse for camera control
    SDL_SetRelativeMouseMode(SDL_FALSE);
    
    SDL_bool Launched = SDL_TRUE;
    SDL_bool needRedraw = SDL_TRUE;
    
    // For frame timing
    Uint32 lastTime = SDL_GetTicks();
    float deltaTime = 0.0f;
    
    while (Launched)
    {
        // Calculate delta time
        Uint32 currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        
        // Auto-rotate if enabled
        if (autoRotate) {
            camera.yaw += deltaTime * 0.5f;
            updateCameraTarget(&camera);
            needRedraw = SDL_TRUE;
            
            // Shift color parameter
            colorShift += deltaTime * 30.0f;
        }
        
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    Launched = SDL_FALSE;
                    break;
                    
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        mouseDown = true;
                        lastMouseX = event.button.x;
                        lastMouseY = event.button.y;
                        
                        if (!keyboardMode) {
                            SDL_SetRelativeMouseMode(SDL_TRUE);
                        }
                    }
                    break;
                    
                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        mouseDown = false;
                        if (!keyboardMode) {
                            SDL_SetRelativeMouseMode(SDL_FALSE);
                        }
                    }
                    break;
                    
                case SDL_MOUSEMOTION:
                    if (mouseDown && !keyboardMode) {
                        // Rotate camera based on mouse movement
                        float sensitivity = 0.003f;
                        camera.yaw += event.motion.xrel * sensitivity;
                        camera.pitch -= event.motion.yrel * sensitivity;
                        
                        // Clamp pitch to avoid gimbal lock
                        if (camera.pitch > 1.5f) camera.pitch = 1.5f;
                        if (camera.pitch < -1.5f) camera.pitch = -1.5f;
                        
                        updateCameraTarget(&camera);
                        needRedraw = SDL_TRUE;
                    }
                    break;
                    
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_ESCAPE:
                            Launched = SDL_FALSE;
                            break;
                            
                        // Toggle control scheme
                        case SDLK_TAB:
                            keyboardMode = !keyboardMode;
                            SDL_SetRelativeMouseMode(SDL_FALSE);
                            printf("Switched to %s control\n", keyboardMode ? "keyboard" : "mouse");
                            break;
                            
                        // Camera rotation (keyboard mode)
                        case SDLK_LEFT:
                            if (keyboardMode) {
                                camera.yaw -= 0.1f;
                                updateCameraTarget(&camera);
                                needRedraw = SDL_TRUE;
                            }
                            break;
                        case SDLK_RIGHT:
                            if (keyboardMode) {
                                camera.yaw += 0.1f;
                                updateCameraTarget(&camera);
                                needRedraw = SDL_TRUE;
                            }
                            break;
                        case SDLK_UP:
                            if (keyboardMode) {
                                camera.pitch += 0.1f;
                                if (camera.pitch > 1.5f) camera.pitch = 1.5f;
                                updateCameraTarget(&camera);
                                needRedraw = SDL_TRUE;
                            }
                            break;
                        case SDLK_DOWN:
                            if (keyboardMode) {
                                camera.pitch -= 0.1f;
                                if (camera.pitch < -1.5f) camera.pitch = -1.5f;
                                updateCameraTarget(&camera);
                                needRedraw = SDL_TRUE;
                            }
                            break;
                        case SDLK_a: {
                            camera.roll -= 0.1f;
                            updateCameraTarget(&camera);
                            needRedraw = SDL_TRUE;
                            break;
                        }
                        case SDLK_e: {
                            camera.roll += 0.1f;
                            updateCameraTarget(&camera);
                            needRedraw = SDL_TRUE;
                            break;
                        }
                            
                        // Camera movement
                        case SDLK_z: { // Forward (W equivalent on QWERTY)
                            Vector3 forward, right, up;
                            getCameraVectors(camera, &forward, &right, &up);
                            camera.position = vectorAdd(camera.position, vectorScale(forward, camera.speed));
                            updateCameraTarget(&camera);
                            needRedraw = SDL_TRUE;
                            break;
                        }
                        case SDLK_s: { // Backward
                            Vector3 forward, right, up;
                            getCameraVectors(camera, &forward, &right, &up);
                            camera.position = vectorSubtract(camera.position, vectorScale(forward, camera.speed));
                            updateCameraTarget(&camera);
                            needRedraw = SDL_TRUE;
                            break;
                        }
                        case SDLK_q: { // Strafe left (A equivalent on QWERTY)
                            Vector3 forward, right, up;
                            getCameraVectors(camera, &forward, &right, &up);
                            camera.position = vectorSubtract(camera.position, vectorScale(right, camera.speed));
                            updateCameraTarget(&camera);
                            needRedraw = SDL_TRUE;
                            break;
                        }
                        case SDLK_d: { // Strafe right
                            Vector3 forward, right, up;
                            getCameraVectors(camera, &forward, &right, &up);
                            camera.position = vectorAdd(camera.position, vectorScale(right, camera.speed));
                            updateCameraTarget(&camera);
                            needRedraw = SDL_TRUE;
                            break;
                        }
                        case SDLK_SPACE: {
                            Vector3 forward, right, up;
                            getCameraVectors(camera, &forward, &right, &up);
                            camera.position = vectorAdd(camera.position, vectorScale(up, camera.speed));
                            updateCameraTarget(&camera);
                            needRedraw = SDL_TRUE;
                            break;
                        }
                        case SDLK_LSHIFT: {
                            Vector3 forward, right, up;
                            getCameraVectors(camera, &forward, &right, &up);
                            camera.position = vectorSubtract(camera.position, vectorScale(up, camera.speed));
                            updateCameraTarget(&camera);
                            needRedraw = SDL_TRUE;
                            break;
                        }
                        
                        // Camera speed
                        case SDLK_EQUALS:  // + key
                            camera.speed *= 1.5f;
                            printf("Camera speed: %.2f\n", camera.speed);
                            break;
                        case SDLK_MINUS:
                            camera.speed /= 1.5f;
                            printf("Camera speed: %.2f\n", camera.speed);
                            break;
                            
                                                    // Quaternion constant C adjustment - Same keys as QWERTY
                            // These are in the same positions on AZERTY keyboard
                        case SDLK_u:
                            C.Iq += 0.01f;
                            needRedraw = SDL_TRUE;
                            break;
                        case SDLK_j:
                            C.Iq -= 0.01f;
                            needRedraw = SDL_TRUE;
                            break;
                        case SDLK_i:
                            C.Rq += 0.01f;
                            needRedraw = SDL_TRUE;
                            break;
                        case SDLK_k:
                            C.Rq -= 0.01f;
                            needRedraw = SDL_TRUE;
                            break;
                        case SDLK_o:
                            C.Jq += 0.01f;
                            needRedraw = SDL_TRUE;
                            break;
                        case SDLK_l:
                            C.Jq -= 0.01f;
                            needRedraw = SDL_TRUE;
                            break;
                        case SDLK_p:
                            C.Kq += 0.01f;
                            needRedraw = SDL_TRUE;
                            break;
                        case SDLK_m: // This key is in the same position on AZERTY
                            C.Kq -= 0.01f;
                            needRedraw = SDL_TRUE;
                            break;
                            
                        // Rendering parameters
                        case SDLK_c:
                            colorMode = (colorMode + 1) % 4;
                            needRedraw = SDL_TRUE;
                            break;
                        case SDLK_r:
                            autoRotate = !autoRotate;
                            printf("Auto-rotate: %s\n", autoRotate ? "ON" : "OFF");
                            break;
                        case SDLK_t:
                            maxIters += 5;
                            printf("Max iterations: %d\n", maxIters);
                            needRedraw = SDL_TRUE;
                            break;
                        case SDLK_g:
                            maxIters = maxIters > 5 ? maxIters - 5 : maxIters;
                            printf("Max iterations: %d\n", maxIters);
                            needRedraw = SDL_TRUE;
                            break;
                        case SDLK_y:
                            colorShift += 20.0f;
                            needRedraw = SDL_TRUE;
                            break;
                        case SDLK_h:
                            colorShift -= 20.0f;
                            needRedraw = SDL_TRUE;
                            break;
                        case SDLK_w:
                            stepSize *= 0.8f;
                            if (stepSize < 0.001f) stepSize = 0.001f;
                            printf("Step size: %.4f\n", stepSize);
                            needRedraw = SDL_TRUE;
                            break;
                        case SDLK_x:
                            stepSize *= 1.25f;
                            if (stepSize > 0.1f) stepSize = 0.1f;
                            printf("Step size: %.4f\n", stepSize);
                            needRedraw = SDL_TRUE;
                            break;
                            
                        // Save screenshot
                        case SDLK_F12:
                            saveScreenshot(rendu, size, size, "julia_quaternion.bmp");
                            break;
                            
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
        }
        
        if (needRedraw) {
            // Ray cast the Julia set
            #pragma omp parallel for collapse(2)
            for (int y = 0; y < size; y++) {
                for (int x = 0; x < size; x++) {
                    // Generate ray for this pixel
                    Ray ray = generateRay(camera, x, y, size, size);
                    
                    // Ray march through the Julia set
                    result_buffer[x + y * size] = rayMarchJulia(ray, maxSteps, stepSize, maxIters, escapeRadius);
                }
            }
            
            // Clear screen
            SDL_SetRenderDrawColor(rendu, 0, 0, 0, 255);
            SDL_RenderClear(rendu);
            
            // Draw pixels
            for (int y = 0; y < size; y++) {
                for (int x = 0; x < size; x++) {
                    RayHit hit = result_buffer[x + y * size];
                    
                    if (hit.hit) {
                        Color color = getColorEnhanced(hit.result, colorMode, colorShift);
                        SDL_SetRenderDrawColor(rendu, color.r, color.g, color.b, color.a);
                        SDL_RenderDrawPoint(rendu, x, y);
                    }
                }
            }
            
            // Update window title with parameters
            char title[256];
            sprintf(title, "Julia Quaternion - C=(%0.2f,%0.2f,%0.2f,%0.2f) - Iters=%d - Mode=%d", 
                    C.Rq, C.Iq, C.Jq, C.Kq, maxIters, colorMode);
            SDL_SetWindowTitle(window, title);
            
            // Present render
            SDL_RenderPresent(rendu);
            
            needRedraw = SDL_FALSE;
        }
        
        // Cap frame rate
        SDL_Delay(1);
    }
    
    /*************************************/
    free(result_buffer);
    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
