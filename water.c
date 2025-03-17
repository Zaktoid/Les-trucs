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


complex_num count_n(complex_num* Array,int size,int x,int y)
{
	complex_num out={0,0};
	for(int k=-1;k<=1;k++)
	{
		for(int l=-1;l<=1;l++)
		{
			if(l||k)
				out=sum(out,Array[x*size+y]);
		}
	}
	return out;
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
	complex_num* Array=malloc(sizeof(complex_num)*hauteur*hauteur);
	complex_num* Array_n=malloc(sizeof(complex_num)*hauteur*hauteur);
    for(int k=0;k<hauteur*hauteur;k++)
    {
        Array[k].Rz=rand()%10;
        Array[k].Iz=rand()%10;
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
            case SDL_MOUSEBUTTONDOWN:
                int x=event.button.x;
                int y=event.button.y;
                Array[x*hauteur+y].Rz+=100;
                Array[x*hauteur+y].Iz+=100;
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
				complex_num C=count_n(Array,hauteur,k,l);
				Array_n[k*hauteur+l]=sum(Array[k*hauteur+l],mult(1.0/10,C));

			}
		}
		for(int k=0;k<hauteur;k++)
			{
				for(int j=0;j<hauteur;j++)
				{
					Array[k*hauteur+j]=Array_n[k*hauteur+j];
                    complex_num v=Array[k*hauteur+j];
                    SDL_SetRenderDrawColor(rendu,module(v),module(v)*10,100,255);
                    SDL_RenderDrawPoint(rendu,k,j);
				}
			}
            SDL_RenderPresent(rendu);

}
/*************************************/
	free(Array);
	free(Array_n);
	SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;

}