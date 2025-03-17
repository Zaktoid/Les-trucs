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
    int size=2000;
    complex_num center={hauteur/2,hauteur/2};
    complex_num* dudes=malloc(size*sizeof(complex_num));
    for(int k=0;k<size;k++)
    {
        dudes[k].Rz=hauteur/2;
        dudes[k].Iz=hauteur/2;
    }
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
    SDL_SetRenderDrawColor(rendu,0,255,255,255);
	int jump=3;
	int n=0;
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
        /*SDL_SetRenderDrawColor(rendu,0,0,0,255);
        SDL_RenderClear(rendu);*/
        int s=((int)(sin(pow(1.1,n))*100))%8;
		printf("%d \n",s);
		jump=1;
        dudes[size-1].Rz=(-jump*(s==0 || s==3 ||s==5)+(int)dudes[size-1].Rz+jump*(s==2 || s==4 ||s==7));
        dudes[size-1].Iz=(-jump*(s==0 || s==1 ||s==2)+(int)dudes[size-1].Iz+jump*(s==5 || s==6 ||s==7));
        for(int t=0;t<size-1;t++)
        {
        	float d=Dist(dudes[t],dudes[t+1]);
            dudes[t].Rz=dudes[t+1].Rz;
            dudes[t].Iz=dudes[t+1].Iz;
            SDL_SetRenderDrawColor(rendu,60+22*d,20+d*30,200,255);
            SDL_RenderDrawPoint(rendu,dudes[t].Rz,dudes[t].Iz);
        }
        SDL_RenderPresent(rendu);
		n=(n+1)%RAND_MAX;
        
		
}
/*************************************/
	SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;

}