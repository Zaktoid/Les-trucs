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
int CountN(int *Cells,int x , int y,int taille)
{
	int s=0;
	for(int i=-1;i<=1;i++)
	{
		for(int j=-1;j<=1;j++)
		{
            if(j||i)
			    s+=Cells[(j+y)*taille+x+i];
		}
	}
	return s; 
}

int main()
{
	srand(time(NULL));
    /**********intialisation d'élements SDL************/
	int hauteur=600;
    int P=5000;
    int T=0;
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
    int* Array_n=malloc(sizeof(int)*hauteur*hauteur);
    for(int k=0;k<hauteur*hauteur;k++)
    {
        Array[k]=Array_n[k]=0;
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
		for(int k=1;k<hauteur-1;k++)
		{
			for(int l=1;l<hauteur-1;l++)
			{
                int s=CountN(Array,k,l,hauteur);
                if(!(rand()%(P-s*(P/8)+1)))
                {
                    Array_n[l*hauteur+k]=Array[l*hauteur+k]+4;
                    T+=4;
                }
			}
		}
    for(int k=0;k<hauteur;k++)
		{
			for(int l=0;l<hauteur;l++)
			{
                Array[l*hauteur+k]=Array_n[l*hauteur+k];
                int s=Array[l*hauteur+k];
                SDL_SetRenderDrawColor(rendu,s*50,s*10,0,255);
                SDL_RenderDrawPoint(rendu,k,l);
            }
        }

        SDL_RenderPresent(rendu);
		
}
/*************************************/
	SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(window);
    free(Array);
    free(Array_n);
	SDL_Quit();
	return EXIT_SUCCESS;

}