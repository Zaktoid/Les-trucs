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
    int T=8;
    int L=hauteur/2;
    complex_num* Tab=malloc(sizeof(complex_num)*(T+1));
    complex_num center={hauteur/2,hauteur/2};
    complex_num A={L,0};
    complex_num cis={cos(2*3.14/(T/2)),sin(2*3.14/(T/2))};
    for(int k=0;k<T/2;k++)
    {
        Tab[k]=sum(A,center);
        A=prod(A,cis); 
    }
    complex_num h={1.0/2.0,0};
    for(int l=0;l<T/2;l++)
    {
        Tab[T/2+l]=prod(sum(Tab[l],Tab[(l+1)%(T/2)]),h);
    }
    Tab[T]=center;
    complex_num C={0.1,0};
    int size=3;
    complex_num* dudes=malloc(size*sizeof(complex_num));
    for(int k=0;k<size;k++)
    {
        dudes[k].Rz=hauteur/2;
        dudes[k].Iz=hauteur/2;
    }
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
                case SDLK_o:
                    C.Rz+=0.01;
                    SDL_SetRenderDrawColor(rendu,0,0,0,255);
                    SDL_RenderClear(rendu);
                    break;
                case SDLK_p:
                    if(C.Rz-0.01)C.Rz-=0.01; 
                    SDL_SetRenderDrawColor(rendu,0,0,0,255);
                    SDL_RenderClear(rendu);
                    break;
                case SDLK_a:
                    Tab[0].Rz+=1;
                    SDL_SetRenderDrawColor(rendu,0,0,0,255);
                    SDL_RenderClear(rendu);
                    break;
				default:
					break;
				}
			default:
				break;
		}}
        int s=rand()%(T);
        complex_num d=prod(sum(Tab[s],opp(dudes[size-1])),C);
        dudes[size-1]=sum(dudes[size-1],d);
        for(int t=0;t<size-1;t++)
        {
            dudes[t].Rz=dudes[t+1].Rz;
            dudes[t].Iz=dudes[t+1].Iz;
            SDL_SetRenderDrawColor(rendu,50*s,200*tanh(40*(t+s)),200*tanh(30*(t+s)),255);
            SDL_RenderDrawPoint(rendu,dudes[t].Rz,dudes[t].Iz);
        }
        SDL_RenderPresent(rendu);
		
}
/*************************************/
	SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;

}