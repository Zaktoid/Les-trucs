#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include "Mathutils.h"


void SDL_Exitwitherror(const char *message)
{
	SDL_Log("Erreur:%s >%s \n",message,SDL_GetError());
	exit(EXIT_FAILURE);
}
void DrawFrac(complex_num P,float sc,float k,float l ,int s,SDL_Renderer* rendu)
{
    if(s!=0)
    {
		complex_num arg={k*cos(atan(P.Iz/P.Rz)),k*sin(atan(P.Iz/P.Rz))};
		for(int j=0;j<l;j++)
		{
			complex_num cis={cos(2*j*3.14/l),sin(2*j*3.14/l)};
			complex_num Q=sum(P,prod(arg,cis));
			SDL_SetRenderDrawColor(rendu,25*s,k*300,s*20,255);
			SDL_RenderDrawLine(rendu,P.Rz,P.Iz,Q.Rz,Q.Iz);
			DrawFrac(Q,sc,k/sc,l,s-1,rendu);
		}
    }
}
int main()
{
	int x=0;
	int y=0;
	float sc=1;
	float ag=4;
	float init=100;
    /**********intialisation d'élements SDL************/
	int hauteur=800;
	SDL_Window *window;
	SDL_Renderer *rendu;
	if(SDL_Init(SDL_INIT_VIDEO))
		SDL_Exitwitherror("failed init");
	window= SDL_CreateWindow("Nom fenêtre",SDL_WINDOWPOS_CENTERED,0,hauteur,hauteur,0);			
	if(!window)
		SDL_Exitwitherror("window creation failed");
	rendu = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);
	if(!rendu)
		SDL_Exitwitherror("renderer failed");
	SDL_bool Launched= SDL_TRUE;

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
				case SDLK_s:
					sc+=0.01;
					break;
				case SDLK_q:
					if(sc-0.1) sc-=0.1;
					break;
				case SDLK_a:
					ag+=0.01;
					break;
				case SDLK_z:
					ag-=0.01;
					break;
				case SDLK_u:
					init+=1;
					break;
				case SDLK_i:
					init-=1;
					break;
				default:
					break;
				}
			default:
				break;
		}}
        SDL_SetRenderDrawColor(rendu,255,0,0,255);
        complex_num a={hauteur/2,hauteur/2};
        DrawFrac(a,sc,init,ag,8,rendu);
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