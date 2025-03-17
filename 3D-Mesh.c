#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#define EPSILON 1e-9
void SDL_Exitwitherror(const char *message)
{
	SDL_Log("Erreur:%s >%s \n",message,SDL_GetError());
	exit(EXIT_FAILURE);
}

////////////////////////////////////////////////////////////////



typedef struct Vertex Vertex;

struct Vertex
{
	float* Coordinates;
	Vertex* Nbs;
};

typedef struct 
{
    float* p;
    float* vect;
    float theta;
}Camera;

Camera* CreatCam(float* p,float* vect, float theta)
{
    float n=pow(vect[0],2)+pow(vect[1],2)+pow(vect[2],2);
    /////Normalize
    if(n!=1)
    {
        for(int i=0;i<3;i++)
            vect[i]=vect[i]/(sqrt(n));
    }
    ////
    Camera* out=malloc(sizeof(Camera));
    out->vect=vect;
    out->p=p;
    out->theta=theta;
    return out;

}

int IsSaw(Camera* C,float* p)
{
    float* v=C->vect;
    float l=(v[0]*p[0]+v[1]*p[1]+v[2]*p[2])
    /(pow(v[0],2)+pow(v[1],2)+pow(v[2],2));
    float d=sqrt(pow(l*v[0]-p[0],2)+pow(l*v[1]-p[1],2)+pow(l*v[2]-p[2],2));
    return (d<=tan(C->theta)*l);
}


Vertex* CreateTriangle(float** points)
{
	Vertex* Out=malloc(sizeof(Vertex)*3);
	for (int i=0;i<3;i++)
	{
		Out[i].Nbs=malloc(sizeof(Vertex)*2);
		Out[i].Nbs[0]=Out[(i+1)%3];
		Out[i].Nbs[1]=Out[(i+2)%3];
		Out[i].Coordinates=points[i];
	}
	return Out;
}





/*void DisplayTriangle(Vertex* Triangle,Camera* C,float K,int L,SDL_Renderer* U,int hauteur)
{
////
}*/



int main()
{
    /**********intialisation d'élements SDL************/
	int hauteur=400;
	SDL_Window *window;
	SDL_Renderer *rendu;
	if(SDL_Init(SDL_INIT_VIDEO))
		SDL_Exitwitherror("failed init");
	window= SDL_CreateWindow("Nom fenêtre",SDL_WINDOWPOS_CENTERED,0,hauteur,hauteur,0);			
	if(!window)
		SDL_Exitwitherror("window creation failed");
	rendu = SDL_CreateRenderer(window,-1,SDL_RENDERER_SOFTWARE);
	if(!rendu)
		SDL_Exitwitherror("renderer failed");
	SDL_bool Launched= SDL_TRUE;
    float p[3]={0,0,0};
    float v[3]={0,1,0};
    Camera* C=CreatCam(p,v,0.936);
    float t[3]={5,3.64,1};
    printf("%d \n",IsSaw(C,t));
    while (Launched)
	{

		
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				Launched= SDL_FALSE;
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
				default:
					break;
				}
			default:
				break;
		}}
		SDL_RenderPresent(rendu);
		SDL_SetRenderDrawColor(rendu,0,0,0,255);
		SDL_RenderClear(rendu);
		
}
/*************************************/
	SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;

}