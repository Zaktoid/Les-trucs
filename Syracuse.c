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

int Syrac(int in)
{
    if(in%2)
    {
        return (3*in+1);
    }
    else 
        return in/2;
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
		for(int k=0;k<hauteur;k+=1)
		{
			for(int l=0;l<hauteur;l+=1)
			{
                int s=l*hauteur+k;
                int i=0;
                while (s>1 && s<=1000000)
                {
                    s=Syrac(s);
                    i++;
                }
                SDL_SetRenderDrawColor(rendu,255*tanh(i),255*tanh(i/100),255*tanh(i/40),255);
                SDL_RenderDrawPoint(rendu,k,l);
			}
		}
        SDL_RenderPresent(rendu);
		
}
/*************************************/
	SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;

}