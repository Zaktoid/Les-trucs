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

int isPrime(int n)
{
	for(int k=2;k<=n/2;k++)
	{
		if(!(n%k))
			return 0;
	}
	return 1;
}

int main()
{
	srand(time(NULL));
    /**********intialisation d'élements SDL************/
	int hauteur=800;
	SDL_Window *window;
	SDL_Renderer *rendu;
    int Prime=1;
    int zoom=1000;
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
    while (Launched)
	{
        SDL_SetRenderDrawColor(rendu,0,0,0,255);
        SDL_RenderClear(rendu);
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
                    zoom+=1;
                    break;
                case SDLK_a:
                    if(zoom-1)
                        zoom-=1;
                    break;
				default:
					break;
				}
			default:
				break;
		}}
        SDL_SetRenderDrawColor(rendu,255,255,255,255);
        while (Prime<=1000000)
        {
                complex_num cis={cos(Prime),sin(Prime)};
                complex_num R={Prime/zoom,0};
                complex_num P=sum(prod(R,cis),C);
                SDL_RenderDrawPoint(rendu,P.Rz,P.Iz);
                Prime+=1;
				printf("%d \n",Prime);
                while (!isPrime(Prime))
                {
                    Prime++;
                }
                
        }
        Prime=1;
        
		SDL_RenderPresent(rendu);
}
/*************************************/
	SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;

}