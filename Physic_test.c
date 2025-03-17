#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <time.h>

typedef struct {
    float* P;
    float* V;
    float* F_R;
    float m;
} Object;

typedef struct QuadTreeNode {
    float x;          
    float y;          
    float width;      
    float total_mass; 
    float cm_x;       
    float cm_y;       
    struct QuadTreeNode* children[4];  
    Object* body;     
} QuadTreeNode;

QuadTreeNode* create_node(float x, float y, float width) {
    QuadTreeNode* node = malloc(sizeof(QuadTreeNode));
    if (!node) return NULL;
    node->x = x;
    node->y = y;
    node->width = width;
    node->total_mass = 0;
    node->cm_x = 0;
    node->cm_y = 0;
    node->body = NULL;
    for(int i = 0; i < 4; i++) {
        node->children[i] = NULL;
    }
    return node;
}

int is_in_bounds(QuadTreeNode* node, Object* body) {
    return body->P[0] >= node->x && 
           body->P[0] < node->x + node->width &&
           body->P[1] >= node->y && 
           body->P[1] < node->y + node->width;
}

void insert_body(QuadTreeNode* node, Object* body) {
    if (!is_in_bounds(node, body)) return;

    if (node->body == NULL && node->children[0] == NULL) {
        node->body = body;
        node->total_mass = body->m;
        node->cm_x = body->P[0];
        node->cm_y = body->P[1];
        return;
    }
    
    if (node->body != NULL) {
        Object* old_body = node->body;
        node->body = NULL;
        
        float half_width = node->width / 2;
        for(int i = 0; i < 4; i++) {
            float offset_x = (i % 2) ? half_width : 0;
            float offset_y = (i < 2) ? 0 : half_width;
            node->children[i] = create_node(node->x + offset_x, node->y + offset_y, half_width);
        }
        
        insert_body(node, old_body);
        insert_body(node, body);
        return;
    }
    
    node->total_mass += body->m;
    node->cm_x = (node->cm_x * (node->total_mass - body->m) + body->P[0] * body->m) / node->total_mass;
    node->cm_y = (node->cm_y * (node->total_mass - body->m) + body->P[1] * body->m) / node->total_mass;
    
    int quad = (body->P[0] >= node->x + node->width/2) + 2*(body->P[1] >= node->y + node->width/2);
    if (!node->children[quad]) {
        float half_width = node->width / 2;
        float new_x = node->x + (quad % 2 ? half_width : 0);
        float new_y = node->y + (quad < 2 ? 0 : half_width);
        node->children[quad] = create_node(new_x, new_y, half_width);
    }
    insert_body(node->children[quad], body);
}

void compute_force(QuadTreeNode* node, Object* body, float theta, float G) {
    if (node == NULL || node->total_mass == 0) return;
    if (node->body == body) return;
    
    float dx = node->cm_x - body->P[0];
    float dy = node->cm_y - body->P[1];
    float dist_sq = dx*dx + dy*dy;
    
    if (dist_sq == 0) return;
    
    float dist = sqrt(dist_sq);
    
    if (node->body != NULL || (node->width / dist < theta)) {
        // Using G as denominator like in original code
        float F = (body->m * node->total_mass) / (G * dist_sq * dist);
        body->F_R[0] += F * dx;
        body->F_R[1] += F * dy;
    } else {
        for(int i = 0; i < 4; i++) {
            compute_force(node->children[i], body, theta, G);
        }
    }
}

void free_quadtree(QuadTreeNode* node) {
    if (node == NULL) return;
    for(int i = 0; i < 4; i++) {
        free_quadtree(node->children[i]);
    }
    free(node);
}

Object* Create_Obj(float* P) {
    Object* out = malloc(sizeof(Object));
    if(!out) return NULL;
    out->P = P;
    out->V = malloc(sizeof(float)*2);
    out->F_R = malloc(sizeof(float)*2);
    if (!out->V || !out->F_R) {
        free(out);
        return NULL;
    }
    out->F_R[0] = out->F_R[1] = 0;
    out->V[0] = out->V[1] = 0;
    return out;
}

void Upgrade(Object* in, float inv_timestep) {
    float dt = 1.0f / inv_timestep;
    float v0_x = in->V[0];
    float v0_y = in->V[1];
    float d_vx = in->F_R[0]/in->m;
    float d_vy = in->F_R[1]/in->m;
    
    in->V[0] += d_vx * dt;
    in->V[1] += d_vy * dt;
    in->P[0] += v0_x * dt;
    in->P[1] += v0_y * dt;
}

void SDL_ExitWithError(const char *message) {
    SDL_Log("Error: %s > %s\n", message, SDL_GetError());
    exit(EXIT_FAILURE);
}

int main() {
    float G = 5000000;
    float theta = 0.5;
    srand(time(0));

    int hauteur = 800;
    int Larg = 800;
    SDL_Window *window;
    SDL_Renderer *rendu;

    if(SDL_Init(SDL_INIT_VIDEO))
        SDL_ExitWithError("Failed init");

    window = SDL_CreateWindow("N-Body Simulation", SDL_WINDOWPOS_CENTERED, 0, hauteur, Larg, 0);
    if(!window)
        SDL_ExitWithError("Window creation failed");

    rendu = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(!rendu)
        SDL_ExitWithError("Renderer failed");

    SDL_bool Launched = SDL_TRUE;

    int nb = 3000;
    Object** Things = malloc(nb * sizeof(Object*));
    if (!Things) exit(EXIT_FAILURE);
    
    for(int k = 0; k < nb; k++) {
        float* P = malloc(2 * sizeof(float));
        if (!P) exit(EXIT_FAILURE);
        P[0] = rand() % hauteur;
        P[1] = rand() % hauteur;
        Things[k] = Create_Obj(P);
        if (!Things[k]) exit(EXIT_FAILURE);
        Things[k]->V[0] = (float)(rand() % 3)/3.0 ;
        Things[k]->m = 0.1;
        Things[k]->V[1] = (float)(rand() % 7)/5.0;
    }

    Things[0]->m = 2000000000;
    Things[0]->P[0] = hauteur/2;
    Things[0]->P[1] = hauteur/2;

    int size = 20;
    float** Array = malloc(nb * size * sizeof(float*));
    if (!Array) exit(EXIT_FAILURE);
    
    for(int k = 0; k < nb*size; k++) {
        Array[k] = malloc(2 * sizeof(float));
        if (!Array[k]) exit(EXIT_FAILURE);
        Array[k][0] = 0;
        Array[k][1] = 0;
    }

    int TR = 0;
    while (Launched) {
        SDL_SetRenderDrawColor(rendu, 0, 0, 0, 255);
        SDL_RenderClear(rendu);

        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    Launched = SDL_FALSE;
                    break;
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_m:
                            Things[nb-1]->m *= 1.3;
                            break;
                        case SDLK_q:
                            Things[nb-1]->m *= -1;
                            break;
                        case SDLK_l:
                            Things[nb-1]->m /= 1.3;
                            break;
                        case SDLK_UP:
                            Things[nb-1]->V[1] -= 0.01;
                            break;
                        case SDLK_DOWN:
                            Things[nb-1]->V[1] += 0.01;
                            break;
                        case SDLK_LEFT:
                            Things[nb-1]->V[0] -= 0.01;
                            break;
                        case SDLK_RIGHT:
                            Things[nb-1]->V[0] += 0.01;
                            break;
                    }
            }
        }

        QuadTreeNode* root = create_node(0, 0, hauteur);
        if (root) {
            for(int i = 0; i < nb; i++) {
                insert_body(root, Things[i]);
            }

            for(int i = 0; i < nb; i++) {
                Things[i]->F_R[0] = Things[i]->F_R[1] = 0;
                compute_force(root, Things[i], theta, G);
            }

            free_quadtree(root);
        }

        for(int k = 0; k < nb; k++) {

            if(k!=0)
				Upgrade(Things[k], 15);
            Array[k*size + TR%size][0] = Things[k]->P[0];
            Array[k*size + TR%size][1] = Things[k]->P[1];
			if(k!=0)
				SDL_SetRenderDrawColor(rendu, (50 + k%200), 50 + 100*Things[k]->V[0], 150 + 100*Things[k]->V[1], 255);
			else
				SDL_SetRenderDrawColor(rendu, 255,255, 255, 255);
            for(int l = 0; l < size; l++) {
                SDL_RenderDrawPoint(rendu, Array[k*size + l][0], Array[k*size + l][1]);
            }
        }

        SDL_RenderPresent(rendu);
        TR++;
    }

    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(window);
    for(int k = 0; k < nb; k++) {
        free(Things[k]->P);
        free(Things[k]->V);
        free(Things[k]->F_R);
        free(Things[k]);
    }
    free(Things);
    for(int k = 0; k < size*nb; k++) {
        free(Array[k]);
    }
    free(Array);
    SDL_Quit();
    return EXIT_SUCCESS;
}