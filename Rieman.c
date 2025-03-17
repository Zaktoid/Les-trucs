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
complex_num Zeta(complex_num in,int iter,int iter2)
{
    complex_num out={0,0};
    for(int k=1;k<iter;k++)
    {
        complex_num a={log(k),0};
        out=sum(out,Inverse(Exp_c(prod(a,in),iter2)));
    }
    return out;
}
int main()
{
	srand(time(NULL));
    /**********intialisation d'élements SDL************/
	int hauteur=700;
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
		for(float k=0;k<hauteur;k+=2)
		{
			for(float l=0;l<hauteur;l+=2)
			{
                complex_num a={-10 + 10*k/hauteur,-10 +10*l/hauteur};
                complex_num s=Zeta(a,10,20);
                SDL_SetRenderDrawColor(rendu,s.Rz*100,s.Iz*50,50,255);
                SDL_RenderDrawPoint(rendu,k,l);
			}
		}
		SDL_RenderPresent(rendu);
}
/*************************************/
    complex_num a={2,0};
    printf("%f,%f",Zeta(a,100,20).Rz,Zeta(a,10,10).Iz);
	SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;

}