#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include "Mathutils.h"

quat C={-0.4, 0.6,0,0};
void SDL_Exitwitherror(const char *message)
{
	SDL_Log("Erreur:%s >%s \n",message,SDL_GetError());
	exit(EXIT_FAILURE);
}
float TestDiverg(int tres,quat (*f)(quat),quat a,double toler)
{
	quat interation=a;
	for(int k=0;k<tres;k++)
	{
		interation=f(interation);
		if(module_q(interation)>toler)
			return k;
	}
	return 0;
}
quat Mendel(quat in)
{
return sum_q(Pow_q(in,2),C);
}

int main(int argc , char **argv)
{
    /**********intialisation d'élements SDL************/
	int hauteur=600;
	SDL_Window *window;
	SDL_Renderer *rendu;
	SDL_Texture *text;
	SDL_Surface *surface;
	float horiz=0;
	float vert=0;
	float z=1;
	int coefR=1;
	int coefG=1;
	int coefB=1;
	int tres=1;
	if(SDL_Init(SDL_INIT_VIDEO))
		SDL_Exitwitherror("failed init");
	window= SDL_CreateWindow("coupe 1",SDL_WINDOWPOS_CENTERED,0,hauteur,hauteur,0);			
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
				case SDLK_ESCAPE:
					Launched=SDL_FALSE;
					break;
				case SDLK_RIGHT:
					horiz+=10;
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
					coefR+=1;
					break;
				case SDLK_g:
					coefG+=1;
					break;
				case SDLK_b:
					coefB+=1;
					break;
				case SDLK_v:
					coefB-=1;
					break;
				case SDLK_e:
					coefR-=1;
					break;
				case SDLK_f:
					coefG-=1;
					break;
				case SDLK_a:
					z+=0.1;
					break;
				case SDLK_q:
					z-=0.1;
					break;
				case SDLK_u:
					C.Iq+=(0.01)/z;
					break;
				case SDLK_j:
					C.Iq-=(0.01)/z;
					break;
				case SDLK_i:
					C.Rq+=(0.01)/z;
					break;
				case SDLK_k:
					C.Rq-=(0.01)/z;
					break;
				case SDLK_o:
					C.Jq+=(0.01)/z;
					break;
				case SDLK_l:
					C.Jq-=(0.01)/z;
					break;
				case SDLK_p:
					C.Kq+=(0.01)/z;
					break;
				case SDLK_m:
					C.Kq-=(0.1)/z;
					break;
				case SDLK_w:
					tres+=1;
					break;
				case SDLK_x:
					tres-=0.1;
					break;
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
					quat a=
					{((double)k-(hauteur/2)+horiz)/(z*(hauteur/4)),
					((double)l-(hauteur/2)+vert)/(z*(hauteur/4)),
					0,
					0,
					};
					float s=TestDiverg(5*z*tres,Mendel,a,5*z*tres);
					if(SDL_SetRenderDrawColor(rendu,(coefR)*s,(coefG)*s,(coefB)*s,255))
						SDL_Exitwitherror("Echec pinceau");
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