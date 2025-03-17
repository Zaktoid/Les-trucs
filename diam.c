#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include "Mathutils.h"
typedef complex_num(*function)(complex_num);
void SDL_Exitwitherror(const char *message)
{
	SDL_Log("Erreur:%s >%s \n",message,SDL_GetError());
	exit(EXIT_FAILURE);
}

int main()
{
    /**********intialisation d'élements SDL************/
	int hauteur=800;
    float z=1;
    float V[3]={50,50,40};
    float horiz=0;
    float vert=0;
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
                    case(SDLK_a):
                        z*=1.1;
                        break;
                    case (SDLK_z):
                        z/=1.1;
                        break;
                    case SDLK_LEFT:
					    horiz-=10;
					    break;
				    case SDLK_UP:
					    vert-=10;
					    break;
				    case SDLK_DOWN:
					    vert+=10;
					    break;
				    case SDLK_r:
					    V[0]+=1;
					    break;
				    case SDLK_g:
					    V[1]+=1;
					    break;
				    case SDLK_b:
					    V[2]+=1;
					    break;
				    case SDLK_v:
					    V[0]-=1;
					    break;
				    case SDLK_e:
					    V[1]-=1;
					    break;
				    case SDLK_f:
					    V[2]-=1;
					    break;
				    default:
					    break;
				}
			default:
				break;
		}}
		for(int k=0;k<hauteur;k+=2)
        {
            for(int j=0;j<hauteur;j+=2)
            {
                float x=(k+horiz)/(z*(hauteur/4));
                float y=(j+vert)/(z*(hauteur/4));
                float s=sqrt((sin(x)-sin(y))*(sin(x)-sin(y))+(cos(x)-cos(y))*(cos(x)-cos(y)));
                SDL_SetRenderDrawColor(rendu,V[0]*s,V[1]*s,V[2]*s,255);
                SDL_RenderDrawPoint(rendu,j,k);

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