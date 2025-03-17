#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include "Mathutils.h"
void SDL_Exitwitherror(const char *message)
{
	SDL_Log("Erreur:%s >%s \n",message,SDL_GetError());
	exit(EXIT_FAILURE);
}

int main()
{
    SDL_Window *wdw;
	SDL_Renderer *rdrd;
    int hauteur=400;
    int largeur=400;
    if(SDL_Init(SDL_INIT_VIDEO))
		SDL_Exitwitherror("Echec init");
	wdw = SDL_CreateWindow("Julia Explorer !",SDL_WINDOWPOS_CENTERED,0,hauteur,largeur,0);			
	if(!wdw)
		SDL_Exitwitherror("Creation fenêtre échouée");
	rdrd = SDL_CreateRenderer(wdw,-1,SDL_RENDERER_ACCELERATED);
	if(!rdrd)
		SDL_Exitwitherror("Echec Rendu");
    float N=1000;
    SDL_SetRenderDrawColor(rdrd,0,0,255,255);
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
            }
        }
            for(int k=0;k<hauteur;k++)    
                {
                    float f=0;
                    for(int j=0;j<N;j++)
                    {
                    f+=1000*sin(pow((float)(k+hauteur)/hauteur,j))/N;
                    }
                    SDL_RenderDrawPoint(rdrd,hauteur-k,largeur-f);
                }
        SDL_RenderPresent(rdrd);
    }
}