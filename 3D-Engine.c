#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <time.h>
#include <omp.h>

#define EPSILON 1e-9
#define WINDOW_SIZE 800        // Default window size
#define FOV 60.0              // Field of view in degrees
#define PI 3.14159265358979323846
#define USE_BOUNDING_BOXES 1  // Enable bounding box optimization

// Error handling
void SDL_ExitWithError(const char *message) {
    SDL_Log("Error: %s > %s\n", message, SDL_GetError());
    exit(EXIT_FAILURE);
}

// Vector operations
typedef struct {
    float x, y, z;
} Vector3;

Vector3 createVector3(float x, float y, float z) {
    Vector3 v = {x, y, z};
    return v;
}

Vector3 addVectors(Vector3 a, Vector3 b) {
    return createVector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vector3 subtractVectors(Vector3 a, Vector3 b) {
    return createVector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vector3 multiplyVector(Vector3 v, float scalar) {
    return createVector3(v.x * scalar, v.y * scalar, v.z * scalar);
}

float dotProduct(Vector3 a, Vector3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vector3 crossProduct(Vector3 a, Vector3 b) {
    return createVector3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

float vectorLength(Vector3 v) {
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vector3 normalizeVector(Vector3 v) {
    float length = vectorLength(v);
    if (length < EPSILON) return v;
    return createVector3(v.x / length, v.y / length, v.z / length);
}

// Triangle structure
typedef struct {
    Vector3 v0, v1, v2;
    Vector3 normal;
    SDL_Color color;
} Triangle;

// Bounding box structure for acceleration
typedef struct {
    Vector3 min;
    Vector3 max;
} BoundingBox;

// Mesh structure
typedef struct {
    Triangle *triangles;
    int triangleCount;
    Vector3 position;      // For transformations
    Vector3 rotation;      // For transformations
    Vector3 scale;         // For transformations
    BoundingBox bounds;    // Bounding box for acceleration
} Mesh;

// Camera
typedef struct {
    Vector3 position;
    Vector3 direction;
    Vector3 up;
    float fov;
} Camera;

// Ray
typedef struct {
    Vector3 origin;
    Vector3 direction;
} Ray;

// Triangle intersection function (Möller–Trumbore algorithm)
// Optimized for speed
float intersectTriangle(Ray ray, Triangle triangle) {
    Vector3 edge1 = subtractVectors(triangle.v1, triangle.v0);
    Vector3 edge2 = subtractVectors(triangle.v2, triangle.v0);
    Vector3 h = crossProduct(ray.direction, edge2);
    float a = dotProduct(edge1, h);
    
    // Culling optimization: if backface culling is desired, can use a > EPSILON instead
    if (fabs(a) < EPSILON) {
        return -1.0f; // Ray is parallel to the triangle
    }
    
    float f = 1.0f / a;
    Vector3 s = subtractVectors(ray.origin, triangle.v0);
    float u = f * dotProduct(s, h);
    
    // Early exit optimization
    if (u < 0.0f || u > 1.0f) {
        return -1.0f;
    }
    
    Vector3 q = crossProduct(s, edge1);
    float v = f * dotProduct(ray.direction, q);
    
    // Early exit optimization
    if (v < 0.0f || u + v > 1.0f) {
        return -1.0f;
    }
    
    float t = f * dotProduct(edge2, q);
    
    // Ensure hit is in front of the ray origin
    if (t > EPSILON) {
        return t;
    }
    
    return -1.0f;
}

// Calculate surface normal of a triangle
Vector3 calculateTriangleNormal(Triangle *triangle) {
    Vector3 edge1 = subtractVectors(triangle->v1, triangle->v0);
    Vector3 edge2 = subtractVectors(triangle->v2, triangle->v0);
    Vector3 normal = crossProduct(edge1, edge2);
    return normalizeVector(normal);
}

// Calculate bounding box for a mesh
BoundingBox calculateBoundingBox(Triangle *triangles, int triangleCount) {
    BoundingBox bounds;
    
    // Initialize with extreme values
    bounds.min = createVector3(INFINITY, INFINITY, INFINITY);
    bounds.max = createVector3(-INFINITY, -INFINITY, -INFINITY);
    
    // Find min and max across all vertices
    for (int i = 0; i < triangleCount; i++) {
        // Check v0
        bounds.min.x = fmin(bounds.min.x, triangles[i].v0.x);
        bounds.min.y = fmin(bounds.min.y, triangles[i].v0.y);
        bounds.min.z = fmin(bounds.min.z, triangles[i].v0.z);
        
        bounds.max.x = fmax(bounds.max.x, triangles[i].v0.x);
        bounds.max.y = fmax(bounds.max.y, triangles[i].v0.y);
        bounds.max.z = fmax(bounds.max.z, triangles[i].v0.z);
        
        // Check v1
        bounds.min.x = fmin(bounds.min.x, triangles[i].v1.x);
        bounds.min.y = fmin(bounds.min.y, triangles[i].v1.y);
        bounds.min.z = fmin(bounds.min.z, triangles[i].v1.z);
        
        bounds.max.x = fmax(bounds.max.x, triangles[i].v1.x);
        bounds.max.y = fmax(bounds.max.y, triangles[i].v1.y);
        bounds.max.z = fmax(bounds.max.z, triangles[i].v1.z);
        
        // Check v2
        bounds.min.x = fmin(bounds.min.x, triangles[i].v2.x);
        bounds.min.y = fmin(bounds.min.y, triangles[i].v2.y);
        bounds.min.z = fmin(bounds.min.z, triangles[i].v2.z);
        
        bounds.max.x = fmax(bounds.max.x, triangles[i].v2.x);
        bounds.max.y = fmax(bounds.max.y, triangles[i].v2.y);
        bounds.max.z = fmax(bounds.max.z, triangles[i].v2.z);
    }
    
    return bounds;
}

// Ray-bounding box intersection test
int intersectBoundingBox(Ray ray, BoundingBox box) {
    float tmin = -INFINITY;
    float tmax = INFINITY;
    
    // For each axis
    for (int i = 0; i < 3; i++) {
        float axis_d, axis_o, axis_min, axis_max;
        
        // Select the appropriate component based on the axis
        if (i == 0) {
            axis_d = ray.direction.x;
            axis_o = ray.origin.x;
            axis_min = box.min.x;
            axis_max = box.max.x;
        } else if (i == 1) {
            axis_d = ray.direction.y;
            axis_o = ray.origin.y;
            axis_min = box.min.y;
            axis_max = box.max.y;
        } else {
            axis_d = ray.direction.z;
            axis_o = ray.origin.z;
            axis_min = box.min.z;
            axis_max = box.max.z;
        }
        
        // Check if ray is parallel to slab
        if (fabs(axis_d) < EPSILON) {
            // Ray is parallel to slab, check if origin is within slab
            if (axis_o < axis_min || axis_o > axis_max) {
                return 0;
            }
        } else {
            // Compute intersection points with slab
            float t1 = (axis_min - axis_o) / axis_d;
            float t2 = (axis_max - axis_o) / axis_d;
            
            // Ensure t1 <= t2
            if (t1 > t2) {
                float temp = t1;
                t1 = t2;
                t2 = temp;
            }
            
            // Update tmin and tmax
            tmin = fmax(tmin, t1);
            tmax = fmin(tmax, t2);
            
            // Check if there's no overlap
            if (tmin > tmax) {
                return 0;
            }
        }
    }
    
    // If we get here, ray intersects all slabs, so it intersects the box
    return 1;
}

// Create sphere as a triangle mesh
Mesh createSphereMesh(Vector3 center, float radius, SDL_Color color, int subdivisions) {
    // Calculate number of vertices and triangles
    int parallels = subdivisions;
    int meridians = subdivisions * 2;
    int triangleCount = parallels * meridians * 2;
    
    Mesh mesh;
    mesh.triangleCount = triangleCount;
    mesh.triangles = (Triangle*)malloc(triangleCount * sizeof(Triangle));
    mesh.position = center;
    mesh.rotation = createVector3(0, 0, 0);
    mesh.scale = createVector3(1, 1, 1);
    
    int index = 0;
    
    // Generate vertices and triangles
    for (int i = 0; i < parallels; i++) {
        for (int j = 0; j < meridians; j++) {
            // Calculate vertices for the current quad
            float u1 = (float)j / meridians;
            float v1 = (float)i / parallels;
            float u2 = (float)(j + 1) / meridians;
            float v2 = (float)(i + 1) / parallels;
            
            float theta1 = u1 * 2.0f * PI;
            float theta2 = u2 * 2.0f * PI;
            float phi1 = v1 * PI;
            float phi2 = v2 * PI;
            
            // Calculate vertex positions
            Vector3 p1 = createVector3(
                center.x + radius * sin(phi1) * cos(theta1),
                center.y + radius * cos(phi1),
                center.z + radius * sin(phi1) * sin(theta1)
            );
            
            Vector3 p2 = createVector3(
                center.x + radius * sin(phi1) * cos(theta2),
                center.y + radius * cos(phi1),
                center.z + radius * sin(phi1) * sin(theta2)
            );
            
            Vector3 p3 = createVector3(
                center.x + radius * sin(phi2) * cos(theta2),
                center.y + radius * cos(phi2),
                center.z + radius * sin(phi2) * sin(theta2)
            );
            
            Vector3 p4 = createVector3(
                center.x + radius * sin(phi2) * cos(theta1),
                center.y + radius * cos(phi2),
                center.z + radius * sin(phi2) * sin(theta1)
            );
            
            // Create two triangles from the quad
            if (i < parallels - 1) {
                mesh.triangles[index].v0 = p1;
                mesh.triangles[index].v1 = p2;
                mesh.triangles[index].v2 = p3;
                mesh.triangles[index].color = color;
                index++;
            }
            
            if (i > 0) {
                mesh.triangles[index].v0 = p1;
                mesh.triangles[index].v1 = p3;
                mesh.triangles[index].v2 = p4;
                mesh.triangles[index].color = color;
                index++;
            }
        }
    }
    
    // Calculate normals for each triangle
    for (int i = 0; i < index; i++) {
        mesh.triangles[i].normal = calculateTriangleNormal(&mesh.triangles[i]);
    }
    
    mesh.triangleCount = index;
    
    // Calculate bounding box for the sphere
    mesh.bounds = calculateBoundingBox(mesh.triangles, mesh.triangleCount);
    
    return mesh;
}

// Create plane as a triangle mesh
Mesh createPlaneMesh(Vector3 center, float width, float depth, SDL_Color color) {
    Mesh mesh;
    mesh.triangleCount = 2;
    mesh.triangles = (Triangle*)malloc(2 * sizeof(Triangle));
    mesh.position = center;
    mesh.rotation = createVector3(0, 0, 0);
    mesh.scale = createVector3(1, 1, 1);
    
    float halfWidth = width / 2.0f;
    float halfDepth = depth / 2.0f;
    
    // Define the four corners of the plane
    Vector3 p1 = createVector3(center.x - halfWidth, center.y, center.z - halfDepth);
    Vector3 p2 = createVector3(center.x + halfWidth, center.y, center.z - halfDepth);
    Vector3 p3 = createVector3(center.x + halfWidth, center.y, center.z + halfDepth);
    Vector3 p4 = createVector3(center.x - halfWidth, center.y, center.z + halfDepth);
    
    // First triangle
    mesh.triangles[0].v0 = p1;
    mesh.triangles[0].v1 = p2;
    mesh.triangles[0].v2 = p3;
    mesh.triangles[0].color = color;
    
    // Second triangle
    mesh.triangles[1].v0 = p1;
    mesh.triangles[1].v1 = p3;
    mesh.triangles[1].v2 = p4;
    mesh.triangles[1].color = color;
    
    // Calculate normals
    mesh.triangles[0].normal = calculateTriangleNormal(&mesh.triangles[0]);
    mesh.triangles[1].normal = calculateTriangleNormal(&mesh.triangles[1]);
    
    // Calculate bounding box
    mesh.bounds = calculateBoundingBox(mesh.triangles, mesh.triangleCount);
    
    return mesh;
}

// Create cube as a triangle mesh
Mesh createCubeMesh(Vector3 center, float size, SDL_Color color) {
    Mesh mesh;
    mesh.triangleCount = 12; // 6 faces * 2 triangles
    mesh.triangles = (Triangle*)malloc(12 * sizeof(Triangle));
    mesh.position = center;
    mesh.rotation = createVector3(0, 0, 0);
    mesh.scale = createVector3(1, 1, 1);
    
    float halfSize = size / 2.0f;
    
    // Define the eight vertices of the cube
    Vector3 v1 = createVector3(center.x - halfSize, center.y - halfSize, center.z - halfSize);
    Vector3 v2 = createVector3(center.x + halfSize, center.y - halfSize, center.z - halfSize);
    Vector3 v3 = createVector3(center.x + halfSize, center.y + halfSize, center.z - halfSize);
    Vector3 v4 = createVector3(center.x - halfSize, center.y + halfSize, center.z - halfSize);
    Vector3 v5 = createVector3(center.x - halfSize, center.y - halfSize, center.z + halfSize);
    Vector3 v6 = createVector3(center.x + halfSize, center.y - halfSize, center.z + halfSize);
    Vector3 v7 = createVector3(center.x + halfSize, center.y + halfSize, center.z + halfSize);
    Vector3 v8 = createVector3(center.x - halfSize, center.y + halfSize, center.z + halfSize);
    
    // Front face
    mesh.triangles[0].v0 = v1; mesh.triangles[0].v1 = v2; mesh.triangles[0].v2 = v3;
    mesh.triangles[1].v0 = v1; mesh.triangles[1].v1 = v3; mesh.triangles[1].v2 = v4;
    
    // Back face
    mesh.triangles[2].v0 = v6; mesh.triangles[2].v1 = v5; mesh.triangles[2].v2 = v8;
    mesh.triangles[3].v0 = v6; mesh.triangles[3].v1 = v8; mesh.triangles[3].v2 = v7;
    
    // Left face
    mesh.triangles[4].v0 = v5; mesh.triangles[4].v1 = v1; mesh.triangles[4].v2 = v4;
    mesh.triangles[5].v0 = v5; mesh.triangles[5].v1 = v4; mesh.triangles[5].v2 = v8;
    
    // Right face
    mesh.triangles[6].v0 = v2; mesh.triangles[6].v1 = v6; mesh.triangles[6].v2 = v7;
    mesh.triangles[7].v0 = v2; mesh.triangles[7].v1 = v7; mesh.triangles[7].v2 = v3;
    
    // Top face
    mesh.triangles[8].v0 = v4; mesh.triangles[8].v1 = v3; mesh.triangles[8].v2 = v7;
    mesh.triangles[9].v0 = v4; mesh.triangles[9].v1 = v7; mesh.triangles[9].v2 = v8;
    
    // Bottom face
    mesh.triangles[10].v0 = v5; mesh.triangles[10].v1 = v6; mesh.triangles[10].v2 = v2;
    mesh.triangles[11].v0 = v5; mesh.triangles[11].v1 = v2; mesh.triangles[11].v2 = v1;
    
    // Set colors and calculate normals for all triangles
    for (int i = 0; i < mesh.triangleCount; i++) {
        mesh.triangles[i].color = color;
        mesh.triangles[i].normal = calculateTriangleNormal(&mesh.triangles[i]);
    }
    
    // Calculate bounding box
    mesh.bounds = calculateBoundingBox(mesh.triangles, mesh.triangleCount);
    
    return mesh;
}

// Load mesh from basic OBJ file
Mesh loadObjMesh(const char *filename, Vector3 position, SDL_Color color) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Failed to open file: %s\n", filename);
        Mesh emptyMesh = {NULL, 0};
        return emptyMesh;
    }
    
    // Count vertices and faces
    char line[128];
    int vertexCount = 0;
    int faceCount = 0;
    
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'v' && line[1] == ' ') vertexCount++;
        if (line[0] == 'f' && line[1] == ' ') faceCount++;
    }
    
    rewind(file);
    
    // Allocate memory
    Vector3 *vertices = (Vector3*)malloc(vertexCount * sizeof(Vector3));
    Mesh mesh;
    mesh.triangleCount = faceCount;
    mesh.triangles = (Triangle*)malloc(faceCount * sizeof(Triangle));
    mesh.position = position;
    mesh.rotation = createVector3(0, 0, 0);
    mesh.scale = createVector3(1, 1, 1);
    
    // Read vertices
    int vIndex = 0;
    int fIndex = 0;
    
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'v' && line[1] == ' ') {
            float x, y, z;
            sscanf(line, "v %f %f %f", &x, &y, &z);
            vertices[vIndex++] = createVector3(x, y, z);
        }
        else if (line[0] == 'f' && line[1] == ' ') {
            int v1, v2, v3;
            // Handle both formats: "f v1 v2 v3" and "f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3"
            char *ptr = line + 2; // Skip "f "
            v1 = atoi(ptr) - 1; // OBJ indices start at 1
            
            // Find next number
            while (*ptr != ' ' && *ptr != '\0') ptr++;
            if (*ptr == '\0') continue;
            ptr++; // Skip space
            v2 = atoi(ptr) - 1;
            
            // Find next number
            while (*ptr != ' ' && *ptr != '\0') ptr++;
            if (*ptr == '\0') continue;
            ptr++; // Skip space
            v3 = atoi(ptr) - 1;
            
            // Create triangle
            mesh.triangles[fIndex].v0 = vertices[v1];
            mesh.triangles[fIndex].v1 = vertices[v2];
            mesh.triangles[fIndex].v2 = vertices[v3];
            mesh.triangles[fIndex].color = color;
            mesh.triangles[fIndex].normal = calculateTriangleNormal(&mesh.triangles[fIndex]);
            fIndex++;
        }
    }
    
    fclose(file);
    free(vertices);
    
    return mesh;
}

// Free mesh resources
void destroyMesh(Mesh *mesh) {
    if (mesh->triangles) {
        free(mesh->triangles);
        mesh->triangles = NULL;
    }
    mesh->triangleCount = 0;
}

// Ray casting function with OpenMP parallelization
void castRay(SDL_Renderer *renderer, Camera camera, Mesh *meshes, int numMeshes) {
    float aspectRatio = (float)WINDOW_SIZE / (float)WINDOW_SIZE;
    float scale = tan(camera.fov * 0.5 * PI / 180.0);
    
    Vector3 right = normalizeVector(crossProduct(camera.direction, camera.up));
    Vector3 up = normalizeVector(crossProduct(right, camera.direction));
    
    // Create a pixel buffer to avoid SDL rendering conflicts in parallel threads
    SDL_Color *pixelBuffer = (SDL_Color*)malloc(WINDOW_SIZE * WINDOW_SIZE * sizeof(SDL_Color));
    
    // Initialize pixel buffer to black
    for (int i = 0; i < WINDOW_SIZE * WINDOW_SIZE; i++) {
        pixelBuffer[i] = (SDL_Color){0, 0, 0, 255};
    }
    
    // Parallelize the outer loop using OpenMP
    #pragma omp parallel
    {
        // Get number of threads and current thread ID
        int numThreads = omp_get_num_threads();
        int threadID = omp_get_thread_num();
        
        // Divide work among threads
        #pragma omp for schedule(dynamic, 16) // Dynamic scheduling with 16-row chunks
        for (int y = 0; y < WINDOW_SIZE; y++) {
            for (int x = 0; x < WINDOW_SIZE; x++) {
                float pixelX = (2.0f * ((x + 0.5f) / WINDOW_SIZE) - 1.0f) * scale * aspectRatio;
                float pixelY = (1.0f - 2.0f * ((y + 0.5f) / WINDOW_SIZE)) * scale;
                
                Ray ray;
                ray.origin = camera.position;
                ray.direction = normalizeVector(
                    addVectors(
                        addVectors(
                            multiplyVector(right, pixelX),
                            multiplyVector(up, pixelY)
                        ),
                        camera.direction
                    )
                );
                
                float closest = INFINITY;
                SDL_Color pixelColor = {0, 0, 0, 255}; // Default black
                
                // Check all meshes
                for (int m = 0; m < numMeshes; m++) {
                    Mesh mesh = meshes[m];
                    
                    // Use bounding box test for early rejection
                    if (USE_BOUNDING_BOXES) {
                        if (!intersectBoundingBox(ray, mesh.bounds)) {
                            continue; // Skip this mesh if ray doesn't hit its bounding box
                        }
                    }
                    
                    // Check all triangles in the mesh
                    for (int t = 0; t < mesh.triangleCount; t++) {
                        float distance = intersectTriangle(ray, mesh.triangles[t]);
                        
                        if (distance > 0 && distance < closest) {
                            closest = distance;
                            pixelColor = mesh.triangles[t].color;
                            
                            // Apply simple lighting (based on normal)
                            Vector3 lightDir = normalizeVector(createVector3(1, 1, -1));
                            float diffuse = fmax(0.2f, dotProduct(mesh.triangles[t].normal, lightDir));
                            
                            pixelColor.r = (Uint8)(pixelColor.r * diffuse);
                            pixelColor.g = (Uint8)(pixelColor.g * diffuse);
                            pixelColor.b = (Uint8)(pixelColor.b * diffuse);
                        }
                    }
                }
                
                if (closest < INFINITY) {
                    // Store the result in the pixel buffer
                    pixelBuffer[y * WINDOW_SIZE + x] = pixelColor;
                }
            }
        }
    } // End of parallel section
    
    // Render the pixel buffer to the screen
    for (int y = 0; y < WINDOW_SIZE; y++) {
        for (int x = 0; x < WINDOW_SIZE; x++) {
            SDL_Color color = pixelBuffer[y * WINDOW_SIZE + x];
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
    
    // Free the pixel buffer
    free(pixelBuffer);
}

// Checkerboard texture function for plane
SDL_Color getCheckerboardColor(Vector3 hitPoint, SDL_Color baseColor, float scale) {
    int checkX = (int)(hitPoint.x * scale) % 2;
    int checkZ = (int)(hitPoint.z * scale) % 2;
    
    SDL_Color result = baseColor;
    
    if ((checkX && !checkZ) || (!checkX && checkZ)) {
        result.r = result.r / 2;
        result.g = result.g / 2;
        result.b = result.b / 2;
    }
    
    return result;
}

// Main function
int main(int argc, char **argv) {
    srand(time(NULL));
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_ExitWithError("Failed to initialize SDL");
    }
    
    SDL_Window *window = SDL_CreateWindow(
        "Triangle Mesh Ray Casting Engine",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_SIZE,
        WINDOW_SIZE,
        0
    );
    
    if (!window) {
        SDL_ExitWithError("Failed to create window");
    }
    
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    if (!renderer) {
        SDL_ExitWithError("Failed to create renderer");
    }
    
    // Setup camera
    Camera camera;
    camera.position = createVector3(0, 2, -8);
    camera.direction = createVector3(0, 0, 1);
    camera.up = createVector3(0, 1, 0);
    camera.fov = FOV;
    
    // Create objects
    int numMeshes = 3;
    Mesh meshes[3];
    
    // Performance optimization: adjust subdivision level based on distance
    printf("Creating objects...\n");
    
    // Create a sphere with triangles (decreased subdivisions for better performance)
    meshes[0] = createSphereMesh(
        createVector3(-2.5, 0, 0),    // center
        1.0f,                         // radius
        (SDL_Color){255, 0, 0, 255},  // red color
        16                            // subdivisions (higher for smoother appearance)
    );
    printf("Sphere created with %d triangles\n", meshes[0].triangleCount);
    
    // Create a cube
    meshes[1] = createCubeMesh(
        createVector3(2.5, 0, 0),     // center
        2.0f,                         // size
        (SDL_Color){0, 255, 0, 255}   // green color
    );
    printf("Cube created with %d triangles\n", meshes[1].triangleCount);
    
    // Create a ground plane
    meshes[2] = createPlaneMesh(
        createVector3(0, -1, 0),         // center
        20.0f,                           // width
        20.0f,                           // depth
        (SDL_Color){200, 200, 200, 255}  // gray color
    );
    printf("Plane created with %d triangles\n", meshes[2].triangleCount);
    
    // Calculate total triangles
    int totalTriangles = 0;
    for (int i = 0; i < numMeshes; i++) {
        totalTriangles += meshes[i].triangleCount;
    }
    printf("Total triangles in scene: %d\n", totalTriangles);
    
    SDL_bool running = SDL_TRUE;
    SDL_Event event;
    
    // Movement speed and rotation speed
    float moveSpeed = 0.1f;
    float rotateSpeed = 0.05f;
    
    // Timing variables for performance measurement
    Uint32 frameStart;
    Uint32 frameTime;
    int frameCount = 0;
    Uint32 lastFPSUpdate = SDL_GetTicks();
    float fps = 0.0f;
    
    // Set the number of threads for OpenMP
    omp_set_num_threads(omp_get_num_procs()); // Use all available cores
    printf("Rendering with %d threads\n", omp_get_num_procs());
    
    // Main loop
    while (running) {
        frameStart = SDL_GetTicks();
        
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = SDL_FALSE;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        running = SDL_FALSE;
                        break;
                    case SDLK_w: // Move forward
                        camera.position = addVectors(camera.position, multiplyVector(camera.direction, moveSpeed));
                        break;
                    case SDLK_s: // Move backward
                        camera.position = subtractVectors(camera.position, multiplyVector(camera.direction, moveSpeed));
                        break;
                    case SDLK_a: { // Strafe left
                        Vector3 right = normalizeVector(crossProduct(camera.direction, camera.up));
                        camera.position = subtractVectors(camera.position, multiplyVector(right, moveSpeed));
                        break;
                    }
                    case SDLK_d: { // Strafe right
                        Vector3 right = normalizeVector(crossProduct(camera.direction, camera.up));
                        camera.position = addVectors(camera.position, multiplyVector(right, moveSpeed));
                        break;
                    }
                    case SDLK_UP: { // Look up
                        float cosAngle = cos(rotateSpeed);
                        float sinAngle = sin(rotateSpeed);
                        Vector3 right = normalizeVector(crossProduct(camera.direction, camera.up));
                        
                        float x = camera.direction.x;
                        float y = camera.direction.y * cosAngle - camera.direction.z * sinAngle;
                        float z = camera.direction.y * sinAngle + camera.direction.z * cosAngle;
                        
                        camera.direction = normalizeVector(createVector3(x, y, z));
                        camera.up = normalizeVector(crossProduct(right, camera.direction));
                        break;
                    }
                    case SDLK_DOWN: { // Look down
                        float cosAngle = cos(-rotateSpeed);
                        float sinAngle = sin(-rotateSpeed);
                        Vector3 right = normalizeVector(crossProduct(camera.direction, camera.up));
                        
                        float x = camera.direction.x;
                        float y = camera.direction.y * cosAngle - camera.direction.z * sinAngle;
                        float z = camera.direction.y * sinAngle + camera.direction.z * cosAngle;
                        
                        camera.direction = normalizeVector(createVector3(x, y, z));
                        camera.up = normalizeVector(crossProduct(right, camera.direction));
                        break;
                    }
                    case SDLK_LEFT: { // Look left
                        float cosAngle = cos(rotateSpeed);
                        float sinAngle = sin(rotateSpeed);
                        
                        float x = camera.direction.x * cosAngle - camera.direction.z * sinAngle;
                        float y = camera.direction.y;
                        float z = camera.direction.x * sinAngle + camera.direction.z * cosAngle;
                        
                        camera.direction = normalizeVector(createVector3(x, y, z));
                        break;
                    }
                    case SDLK_RIGHT: { // Look right
                        float cosAngle = cos(-rotateSpeed);
                        float sinAngle = sin(-rotateSpeed);
                        
                        float x = camera.direction.x * cosAngle - camera.direction.z * sinAngle;
                        float y = camera.direction.y;
                        float z = camera.direction.x * sinAngle + camera.direction.z * cosAngle;
                        
                        camera.direction = normalizeVector(createVector3(x, y, z));
                        break;
                    }
                }
            }
        }
        
        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        // Render the scene
        castRay(renderer, camera, meshes, numMeshes);
        
        // Update the screen
        SDL_RenderPresent(renderer);
        
        // Calculate and display FPS
        frameTime = SDL_GetTicks() - frameStart;
        frameCount++;
        
        if (SDL_GetTicks() - lastFPSUpdate >= 1000) {
            fps = frameCount * 1000.0f / (SDL_GetTicks() - lastFPSUpdate);
            frameCount = 0;
            lastFPSUpdate = SDL_GetTicks();
            
            char title[64];
            sprintf(title, "Triangle Mesh Ray Casting Engine - FPS: %.2f", fps);
            SDL_SetWindowTitle(window, title);
        }
    }
    
    // Free resources
    for (int i = 0; i < numMeshes; i++) {
        destroyMesh(&meshes[i]);
    }
    
    // Cleanup and exit
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return EXIT_SUCCESS;
}
