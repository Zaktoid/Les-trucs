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

int main()
{
	srand(time(NULL));
    /**********intialisation d'élements SDL************/
	int hauteur=800;
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
    int* Array=malloc(sizeof(int)*hauteur*hauteur);
    int F[2];
    F[0]=F[1]=hauteur/2;
    complex_num Dir={1,0};
    complex_num T={0,1};
    for(int k=0;k<hauteur*hauteur;k++)
        {Array[k]=0;}
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
        if(Array[F[0]*hauteur +F[1]]==0)
            {
                Array[F[0]*hauteur +F[1]]=1;
                SDL_SetRenderDrawColor(rendu,255,255,255,255);
                SDL_RenderDrawPoint(rendu,F[0],F[1]);
                Dir=prod(Dir,T);
                F[0]=((F[0]<=0)*hauteur +(F[0] +(int)Dir.Rz))%hauteur;
                F[1]=((F[1]<=0)*hauteur +(F[1] +(int)Dir.Iz))%hauteur;
            }
		if(Array[F[0]*hauteur +F[1]]==1)
            {
                Array[F[0]*hauteur +F[1]]=0;
                SDL_SetRenderDrawColor(rendu,0,0,0,255);
                SDL_RenderDrawPoint(rendu,F[0],F[1]);
                Dir=prod(Dir,Inverse(T));
                F[0]=((F[0]<=0)*hauteur +(F[0] +(int)Dir.Rz))%hauteur;
                F[1]=((F[1]<=0)*hauteur+ (F[1] +(int)Dir.Iz))%hauteur;
            }
        SDL_RenderPresent(rendu);
}
/*************************************/
	SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;

}