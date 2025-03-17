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
	int hauteur=200;
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
    SDL_SetRenderDrawColor(rendu,255,255,255,255);
    int t=0;
	int Tab[hauteur][hauteur];
	for(int i=0;i<hauteur;i++)
		{
			for(int j=0;j<hauteur;j++)
				Tab[i][j]=0;
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
                break;
            case SDL_MOUSEBUTTONDOWN:
                t=1;
                break;
            case SDL_MOUSEMOTION:
                int x=event.button.x;
                int y=event.button.y;
                if(t)
                    SDL_RenderDrawPoint(rendu,x,y);
					Tab[x][y]=1;
                break;
            case SDL_MOUSEBUTTONUP:
                t=0;
                break;
			default:
				break;
		}}
        SDL_RenderPresent(rendu);


}
/*************************************/
	FILE *image=fopen("Image","w");
	fprintf(image,"AS: %d,%d \n",hauteur,hauteur);
	for(int i=0;i<hauteur;i++)
	{
		for(int j=0;j<hauteur;j++)
		{
			if(Tab[i][j])
				fprintf(image,"0");
			else 
				fprintf(image,"1");
		}
		fprintf(image,"\n");
	}
	fclose(image); 
	SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;

}