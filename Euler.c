#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <time.h>
#include "Mathutils.h"


void SDL_Exitwitherror(const char *message)
{
	SDL_Log("Erreur:%s >%s \n",message,SDL_GetError());
	exit(EXIT_FAILURE);
}

float Module(float* t,int dim)
{
    float out=0;
    for (int k=0;k<dim;k++)
    {
        out+=t[k]*t[k];
    }
    return sqrt(out);
    
}

int main()
{
	srand(time(NULL));
    /**********intialisation d'élements SDL************/
	int hauteur=800;
	SDL_Window *window;
	SDL_Renderer *rendu;
    float z=1;
    float horiz=0;
    float vert=0;
    float M=3;
    float t_0=1;
    int DIM=1;
    float C=2;
    float* out=malloc(sizeof(float)*2);
    float u=1;
    float* x_0=malloc(sizeof(float)*DIM);
    x_0[0]=0.84;
	if(SDL_Init(SDL_INIT_VIDEO))
		SDL_Exitwitherror("failed init");
	window= SDL_CreateWindow("Nom fenêtre",SDL_WINDOWPOS_CENTERED,0,hauteur,hauteur,0);			
	if(!window)
		SDL_Exitwitherror("window creation failed");
	rendu = SDL_CreateRenderer(window,-1,SDL_RENDERER_SOFTWARE);
	if(!rendu)
		SDL_Exitwitherror("renderer failed");
	SDL_bool Launched= SDL_TRUE;
    while (Launched)
	{
        SDL_SetRenderDrawColor(rendu,0,0,0,255);
        SDL_RenderClear(rendu);
        SDL_SetRenderDrawColor(rendu,255,255,255,255);
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
                case SDLK_z:
                    z+=1;
                    break;
                case SDLK_q:
                    if(z-1)z-=1;
                    break;
                case SDLK_RIGHT:
                    horiz+=1.0;
                    break;
                case SDLK_LEFT:
                    horiz-=1.0;
                    break;
                case SDLK_UP:
                    vert+=1.0;
                    break;
                case SDLK_DOWN:
                    vert-=1.0;
                    break;
                case SDLK_m:
                    M++;
                    break;
                case SDLK_n:
                    M--;
                    break;
                case SDLK_c:
                    C+=1;
                    break;
                case SDLK_t:
                    x_0[0]+=0.01;
                    break;
                case SDLK_g:
                    x_0[0]-=0.01;
                    break;
                case SDLK_f:
                    t_0-=0.01;
                    break;
                case SDLK_h:
                    t_0+=0.01;
                    break;
                case SDLK_o:
                    u+=0.01;
                    break;
                case SDLK_p:
                    u-=0.01;
                    break;
				default:
					break;
				}
			default:
				break;
		}}
    float* f(float t,float* x,int dim)
    {
    float* o=malloc(sizeof(float)*dim);
    o[0]=t*t;
	return o;
    }
	float H=20;
	float h=H/M;
	float* T=malloc(sizeof(float*)*M);
	T[0]=t_0;
	for(int m=1;m<M;m++)
	{

            T[m]=T[0]+h*m;

	}
    float **X=malloc(sizeof(float*)*M);
        for(int j=0;j<DIM;j++)
        {
            X[j]=malloc(sizeof(float)*2);
        }
    X[0]=x_0;
        for(int m=1;m<M;m++)
	    {
		X[m]=malloc(sizeof(float)*DIM);
        for(int k=0;k<DIM;k++)
        {
            X[m][k]=X[m-1][k]+h*f(T[m-1],X[m-1],DIM)[k];
            if(X[m][k]<=-1000)
            {
                X[m][k]=-1000;
            }
            if(X[m][k]>=1000)
            {
                X[m][k]=1000;
            }
        }
	    }
        for(float j=0;j<hauteur;j+=10)
        {
            for(float k=0;k<hauteur;k+=10)
            {
                float in[1];
                in[0]=(j+vert)/z;
                out=f((k+horiz)/z,in,2);
                float alpha=atan(out[0]);
                SDL_SetRenderDrawColor(rendu,100*cos(alpha),100*sin(alpha),200,0);
                SDL_RenderDrawLine(rendu,k,j,k+C*cos(alpha),j+C*sin(alpha));
                SDL_SetRenderDrawColor(rendu,255,255,255,255);
                SDL_RenderDrawPoint(rendu,k+C*cos(alpha),j+C*sin(alpha));
            }}
    SDL_SetRenderDrawColor(rendu,255,255,255,255);
    for(int k=0;k<M-1;k++)
    {
        SDL_RenderDrawLine(rendu,T[k]*z-horiz,(X[k][0])*z-vert,
        T[k+1]*z-horiz,(X[k+1][0])*z-vert);
    }

    SDL_RenderPresent(rendu);
    free(X);
    free(T);

		
}
free(out);
/*************************************/
	SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;

}