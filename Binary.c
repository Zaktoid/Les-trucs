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


int main(int argc , char **argv)
{
	srand(time(NULL));
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
    int A=0;
    while (Launched)
	{
	SDL_SetRenderDrawColor(rendu,255,0,0,255);
	for(int i=0;i<hauteur;i++)
	{
		float k=5*(i+A);
		int n=0;
		while (k>=1)
		{
			if((int)k%2) SDL_RenderDrawPoint(rendu,hauteur -n,i);
			n++;
			k=k/2.0;
		}
		
	}
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
                case SDLK_a:
                    A+=100000;
                    SDL_SetRenderDrawColor(rendu,0,0,0,255);
                    SDL_RenderClear(rendu);

				default:
					break;
				}
			default:
				break;
		}}
		SDL_RenderPresent(rendu);


}
/*************************************/

	SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;

}