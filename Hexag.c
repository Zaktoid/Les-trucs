#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <time.h>
#include "Mathutils.h"

struct Cell
{
    struct Tile** Nbs;
    int val;
};
typedef struct Tile Tile;



void SDL_Exitwitherror(const char *message)
{
	SDL_Log("Erreur:%s >%s \n",message,SDL_GetError());
	exit(EXIT_FAILURE);
}

int main()
{
	srand(time(NULL));
	Tile* T=malloc(sizeof(Tile));
	InitPaving(1,T);
	T->val=3;
	printf("%d \n",T->Nbs[0]->Nbs[3]->val);
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

			}
		}
		
}
/*************************************/
	SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;

}