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

float f(float t,float x)
{
	return t*sin(x);
}

int main(int argc , char **argv)
{
	srand(time(NULL));
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
	complex_num C={hauteur/2,hauteur/2};
	float M=1000;
	float H=100;
	float h=M/H;
	float t_0=0;
	float x_0=0;
	float* T=malloc(sizeof(float)*M);
	T[0]=t_0;
	for(int m=1;m<M;m++)
	{
		T[m]=T[0]+h*M;
	}
	float** X=malloc(sizeof(float*)*M);
	for(int k=0;k<M;k++)
    {
        X[k]=malloc(sizeof(float)*M);
    }
	X[0][1]=X[0][0]=1;
	for(int m=1;m<M;m++)
	{
		X[m][0]=X[m-1][0]+h*f(T[m],X[m]);
	}
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
		for(int k=0;k<hauteur;k++)
		{
			for(int l=0;l<hauteur;l++)
			{
				complex_num T={k,l};
				float s=Dist(T,C);
				SDL_SetRenderDrawColor(rendu,tanh(s)*255,tanh(s)*255,tanh(s)*255,255);
			}
		}
		
}
/*************************************/
	SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;

}