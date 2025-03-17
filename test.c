#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <math.h>
#include "Mathutils.h"


void SDL_Exitwitherror(const char *message)
{
	SDL_Log("Erreur:%s >%s \n",message,SDL_GetError());
	exit(EXIT_FAILURE);
}
double Det(double* Matrix,int size)
{
	if(size==4)
	{
		double out=Matrix[0]*Matrix[3]-Matrix[1]*Matrix[2];
		return out;
	}
	
}
void Display_ComplexFunc(complex_num(*f)(complex_num),SDL_Renderer* rendu,int hauteur,int Larg,int scale)
{
    for(int k=0;k<hauteur;k++)
    {
        for(int j=0;j<Larg;j++)
        {
            complex_num Z={k,j};
            complex_num Z_0=f(Z);
            float D=module(Z_0)/scale;
            SDL_SetRenderDrawColor(rendu,Z_0.Rz,20*D,Z_0.Iz,255);
            SDL_RenderDrawPoint(rendu,k,j);
        }
    }
}
int main(int argc , char **argv)
{
	int x=0;
	int y=0;
    short follow=0;
    /**********intialisation d'élements SDL************/
	int hauteur=800;
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
    /***************************************************/
    int n=6;
    slider* S=malloc(sizeof(slider)*n);
    for(int k=0;k<n;k++)
    {
        S[k].actu=0;
        S[k].inf=0;
        S[k].sup=100;
        S[k].length=100;
        S[k].X=0;
        S[k].Y=k*hauteur/n;
        S[k].targeted=0;
    }
	SDL_bool Launched= SDL_TRUE;
    while (Launched)
	{
        SDL_RenderPresent(rendu);
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				Launched= SDL_FALSE;
				break;
			case SDL_MOUSEBUTTONDOWN:
                x=event.button.x;
                y=event.button.y;
                for(int k=0;k<n;k++)
                {
                    if((x>=S[k].X && x<=S[k].X+S[k].length)&& (y<=S[k].Y+5 && y>=S[k].Y-5))
                    {
                        S[k].targeted=1;
                    }
                }
				break;
            case SDL_MOUSEMOTION:
                for(int k=0;k<n;k++)
                    {
                        if(S[k].targeted)
                        {
                            float r=S[k].sup*(event.button.x-S[k].X)/S[k].length +S[k].inf;
                            if(r>=S[k].inf && r<=S[k].sup)
                            {
                                S[k].actu=r;
                            }
                        }
                    }
                break;
            case SDL_MOUSEBUTTONUP:
                for(int k=0;k<n;k++)
                    {
                        S[k].targeted=0;
                    }
                break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
                    break;
				default:
					break;
				}
			default:
				break;
		}}
    complex_num T(complex_num In)
    {
        complex_num r={1.0,0};
        for(int k=0;k<n;k+=2)
        {
            complex_num C={S[k].actu,S[k+1].actu};
            r=prod(r,sum(In,C));
        }
        return sum(r,In);
    }
        SDL_SetRenderDrawColor(rendu,0,0,0,255);
        SDL_RenderClear(rendu);
        Display_ComplexFunc(T,rendu,hauteur,hauteur,10);
        for(int k=0;k<n;k++)
            Draw_slider(S[k],rendu);

		
}
/*************************************/
	SDL_DestroyRenderer(rendu);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;

}