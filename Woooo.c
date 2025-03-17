#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <time.h>


void SDL_Exitwitherror(const char *message)
{
	SDL_Log("Erreur:%s >%s \n",message,SDL_GetError());
	exit(EXIT_FAILURE);
}

int main(int argc , char **argv)
{
	srand(time(NULL));
    /**********intialisation d'élements SDL************/
	int hauteur=400;
	int* array=malloc(hauteur*hauteur*sizeof(int));
	for(int k=0;k<hauteur*hauteur;k++)
	{
		int s=rand();
		array[k]=s%2;
	}
	SDL_Window *window;
	SDL_Renderer *rendu;
	SDL_Texture *text;
	SDL_Surface *surface;
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
				int p=array[k*hauteur+l];
				int s=0;
				for(int j=-1;j<=1;j++)
				{
					for(int i=0;i<=1;i++)
					{
						if(i!=k || j!=l)
							if(array[(k+i)*hauteur+l+j]==p)
								s+=1;
					}
				}
				if(s>=4) 
                array[k*hauteur+l]=1-p;
				array[(k+1)*hauteur+l-1]=1-array[(k+1)*hauteur+l-1];
                SDL_SetRenderDrawColor(rendu,array[k*hauteur+l]*100,(1-array[k*hauteur+l])*100,0,255);
                SDL_RenderDrawPoint(rendu,k,l);
				
			}
		}
        SDL_RenderPresent(rendu);
		
}
/*************************************/
	SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(window);
	free(array);
	SDL_Quit();
	return EXIT_SUCCESS;

}